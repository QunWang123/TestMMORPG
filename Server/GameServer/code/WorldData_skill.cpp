#include "WorldData.h"
#include "UserManager.h"
#include "AppGlobal.h"
#include "WorldMap.h"
#include "WorldTools.h"
#include "CommandData.h"
#include "WorldScript.h"
#include "WorldBC.h"
#include "RobotManager.h"

namespace app
{
	S_ROLE_STAND_SKILL_BASE* S_USER_BASE::skill_IsSafe(const u8 index)
	{
		if (index >= MAX_SKILL_COUNT) return nullptr;
		S_ROLE_STAND_SKILL_BASE* skill = &this->role.stand.myskill.skill[index];
		if (!skill->isT()) return nullptr;
		return skill;
	}

	bool S_USER_BASE::skill_IsPublicTime()
	{
		s32 value = global_gametime - this->tmp.temp_SkillPublicTime;
		if (value > 0 && value < USER_SKILL_PUBLICTIME)
		{
			return false;
		}
		return true;
	}

	bool S_USER_BASE::skill_IsColdTime(const u8 index, const u32 cdtime)
	{
		if (index >= MAX_SKILL_COUNT) return false;
		s32 value = global_gametime - this->tmp.temp_SkillColdTime[index];
		if (value > 0 && value < cdtime)
		{
			return false;
		}
		return true;
	}

	// ��ʼ����	
	// 1.��ʼ���ż���
	s32 S_USER_BASE::skill_start(void* data, void* sk)
	{
		S_SKILL_ROLE* d = (S_SKILL_ROLE*)data;
		script::SCRIPT_SKILL_BASE* skillscript = (script::SCRIPT_SKILL_BASE*)sk;

		S_WORLD_MAP* map = __World->getMap(this->role.base.status.mapid);
		if (map == NULL) return 3013;

		S_VECTOR3 targetpos;
		
		switch (d->lock_type)
		{
		case N_ROLE:
		{
			S_USER_BASE* enemy = __UserManager->findUser(d->lock_index, this->node.layer);
			if (enemy == NULL) return 3013;
			targetpos = enemy->role.base.status.pos;
		}
			break;
		case N_MONSTER:
		case N_PET:
		case N_NPC:
		{
			S_ROBOT* enemy = __RobotManager->findRobot(d->lock_index, this->node.layer);
			if (enemy == NULL) return 3013;
			TS_Tools::gridToPos(&enemy->bc.grid_pos, &targetpos,
				&map->leftpos);
			// �������z���Ҫ���ߵͲ�Ļ�������
			// ������Ȼ�������ˣ�����ʵͬһ�߶�δ�ز��ɰѣ�
			targetpos.z = this->role.base.status.pos.z;
		}
			break;
		case N_FREE:
			targetpos = d->targetpos;
			break;
		default:
			LOG_MESSAGE("OnSkillLock err ...line:%d \n", __LINE__);
			return 3014;
		}

		s32 distance = TS_Tools::getDistance(&this->role.base.status.pos, &targetpos);

		// �Ͼ����е��ӳٵģ�����Ҫ�����ӳ�	
		if (distance > skillscript->distance + 30)
		{
			LOG_MESSAGE("skill distance err... %d-%d [%d-%d-%d] [%d-%d-%d] line:%d \n",
				distance, skillscript->distance, this->role.base.status.pos.x,
				this->role.base.status.pos.y, this->role.base.status.pos.z,
				targetpos.x, targetpos.y, targetpos.z, __LINE__);
			return 3015;
		}

		// ����һ�����еļ���
		auto run = this->tmp.temp_SkillRun.findFree();
		if (run == NULL) return 3016;
		
		run->targetpos = targetpos;
		run->lock_type = d->lock_type;
		run->lock_index = d->lock_index;
		run->skill_index = d->skillindex;		// d->skillindex��ָ�ģ���ʵ���run->skill_index��run->index�ô�����

		run->skill_id = skillscript->id;
		run->skill_level = skillscript->level;
		run->damagecount = skillscript->damagecount;
		run->runingtime = skillscript->runingtime;
		run->continued = skillscript->continued;
		run->continued_waittime = skillscript->waittime;
		run->temp_continuedtime = 10000000;
		run->temp_calpos = this->role.base.status.pos;

		run->flytime = skillscript->flytime;	// ��������ʱ��
		if(run->flytime > 10000)	// Զ�̹�������Ҫ����
		{
			run->flytime -= 10000;
			run->flytime += (distance * 0.5);	// ���300cm���ͼ��300����ִ��
		}

		// ����CDʱ��
		this->tmp.temp_SkillPublicTime = global_gametime;
		this->tmp.temp_SkillColdTime[d->skillindex] = global_gametime;

		// �㲥
		TS_Broadcast::bc_RoleSkill(this, run);

		// ��̵��ƶ�����
		if (skillscript->id == 1006)
		{
			// ���������ƶ�
			bool isupdata = false;
			
			this->moveWorld(&targetpos, isupdata);

			if (isupdata)
			{
				// �㲥�뿪����
				TS_Broadcast::bc_RoleMoveLeaveBig(&this->bc.edgeold, this);
			}
		}

		return 0;
	}

	s32 onSkill_AtkTarget(S_USER_BASE* user, S_TEMP_SKILL_RUN_BASE* run, script::SCRIPT_SKILL_BASE* skillscript)
	{
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return -1;

		// ����һ���ɹ���������
		S_RECT_BASE rect;
		// Ŀ�����, Ҫת�ɴ�����
		S_GRID_BASE target_grid;
		// Ŀ���������λ��
		s32 targetindex = -1;
		// �˺��������
		s32 damagecount = skillscript->damagecount;

		u32 distance = 0;

		void* enemy = NULL;
		S_USER_BASE* enemy_user = NULL;
		S_ROBOT*enemy_robot = NULL;
		// �ǲ��ǳ����Լ���
		if (run->continued <= 1)
		{
			// ������Ŀ��
			if (run->lock_type != N_FREE)
			{
				// target_grid�ڴ˴����Ѿ��ɹ���ȡ��
				enemy = TS_Tools::findSprite(&user->bc.edge, run->lock_type, run->lock_index, 
					user->node.layer, user->role.base.status.mapid, &target_grid);
				if (enemy == NULL) return -1;
				
				// �����˺�
				// findSprite��ʱ������һ��enemy,��������һ�Σ��˷�����
				// Ӧ����һ���µ�setSkillDamage����������enemy��ȥת������
				TS_Tools::setSkillDamage(user, run->lock_type, enemy, run, skillscript);
				// ����buff
				TS_Tools::buff_skill_random(enemy, skillscript, run->lock_type);

				targetindex = run->lock_index;
				damagecount -= 1;
				if (damagecount <= 0) return 0;
			}
		
			// ȷ���ܻ���λ�ã�����������񣬵����ﶼ˵��damagecount>=1�����ܼ���������Ŀ������˺�
			switch (run->lock_type)
			{
			case N_ROLE:
				enemy_user = (S_USER_BASE*)enemy;
				run->targetpos = enemy_user->role.base.status.pos;			// ����Ŀ�����ֻ꣨��Ϊ�˷�ֹĿ���ƶ���
				break;
			case N_MONSTER:
			case N_PET:
			case N_NPC:
				enemy_robot = (S_ROBOT*)enemy;
				TS_Tools::gridToPos(&enemy_robot->bc.grid_pos, &run->targetpos, &map->leftpos);// ����Ŀ�����ֻ꣨��Ϊ�˷�ֹĿ���ƶ���
				break;
			default:
				// û������Ŀ��Ļ���������������target_grid��������Ŀ��Ļ�target_grid���Ѿ����ú�
				TS_Tools::posToGrid(&target_grid, &run->targetpos, &map->leftpos);
				break;
			}

			// ȷ��������С����λ��
			if (skillscript->type == E_SKILL_USE_TYPE_SELF)
			{
				target_grid = user->bc.grid_pos;
			}
		}
		else
		{
			// �����Լ���
			TS_Tools::posToGrid(&target_grid, &run->targetpos, &map->leftpos);
			if (skillscript->type == E_SKILL_USE_TYPE_SELF)
			{
				if (skillscript->isfollow != 0)
				{
					target_grid = user->bc.grid_pos;
				}
			}
		}

		// ����ȷ���˹�����С���ӣ���Ҫȷ������ӣ���ȷ����Щ��һ��ܵ�Ӱ��
		// ����������ܰ뾶���Ƕ��ٸ�С����,����ȡ��
		s32 glen = ceil(skillscript->radius / C_WORLDMAP_ONE_GRID);
		if (!map->Get_Redius_View(&target_grid, glen, &rect)) return -3;

		LOG_MESSAGE("skill atk rect %lld, %d-%d-%d-%d\n", user->mem.id, rect.left, rect.top, rect.right, rect.bottom);

		for (u32 row = rect.top; row <= rect.bottom; row++)
		{
			for (u32 col = rect.left; col <= rect.right; col++)
			{
				if (damagecount <= 0) break;
				if (!map->IsRect(row, col)) continue;

				S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;

				while (node != nullptr)
				{
					if (damagecount <= 0) break;
					
					if (node->index == targetindex || 
						(node->type == N_ROLE && node->index == user->node.index) ||
						node->layer != user->node.layer)
					{
						node = node->downnode;
						continue;
					}

					enemy = TS_Tools::findSprite(&user->bc.edge, node->type, node->index,
						user->node.layer, user->role.base.status.mapid);

					if (enemy == NULL)
					{
						node = node->downnode;
						continue;
					}

					// �жϾ���
					bool isatk = TS_Tools::isInRadius(user, node->type, enemy, &run->targetpos, skillscript);
					if (!isatk)
					{
						node = node->downnode;
						continue;
					}

					// �����˺�
					TS_Tools::setSkillDamage(user, node->type, enemy, run, skillscript);
					//����buff
					TS_Tools::buff_skill_random(enemy, skillscript, node->type);

					damagecount -= 1;
					node = node->downnode;
				}
			}
		}
		return 0;
	}

	s32 onSkill_AtkDir(S_USER_BASE* user, S_TEMP_SKILL_RUN_BASE* run, script::SCRIPT_SKILL_BASE* skillscript)
	{
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return -1;

		s32 damagecount = skillscript->damagecount;
		s32 targetindex = run->lock_index;

		void* enemy = TS_Tools::findSprite(&user->bc.edge, run->lock_type, run->lock_index,
			user->node.layer, user->role.base.status.mapid);

		if (enemy != NULL)
		{
			// findSprite��ʱ������һ��enemy,��������һ�Σ��˷�����
			// Ӧ����һ���µ�setSkillDamage����������enemy��ȥת������
			TS_Tools::setSkillDamage(user, run->lock_type, enemy, run, skillscript);
			TS_Tools::buff_skill_random(enemy, skillscript, run->lock_type);
			damagecount -= 1;
			if (damagecount <= 0)return 0;
		}

		// ����������ҵ�����
		S_RECT_BASE rect;
		S_VECTOR3 atkpos = user->role.base.status.pos;
		s32 distance = skillscript->distance;
		// ��������⴦��
		if (run->skill_id == 1006)
		{
			atkpos = run->temp_calpos;
			distance = TS_Tools::getDistance(&atkpos, &run->targetpos);
		}

		s32 glen = ceil(skillscript->radius / C_WORLDMAP_ONE_GRID);
		if (!map->Get_Redius_View(&user->bc.grid_pos, glen, &rect)) return -3;

		// ����㵽ֱ�ߵı�����
		f32 value = TS_Tools::getPointToLine_3D(&atkpos, &run->targetpos);


		for (u32 row = rect.top; row <= rect.bottom; row++)
		{
			for (u32 col = rect.left; col <= rect.right; col++)
			{
				if (damagecount <= 0) break;
				if (!map->IsRect(row, col)) continue;

				S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;

				while (node != nullptr)
				{
					if (damagecount <= 0) break;

					if (node->index == targetindex ||
						(node->type == N_ROLE && node->index == user->node.index) ||
						node->layer != user->node.layer)
					{
						node = node->downnode;
						continue;
					}

					enemy = TS_Tools::findSprite(&user->bc.edge, node->type, node->index,
						node->layer, user->role.base.status.mapid);
					if (enemy == NULL)
					{
						node = node->downnode;
						continue;
					}
					// ȷ���ڵ��λ��
					S_VECTOR3 enemy_pos;
					switch (node->type)
					{
					case N_ROLE:
					{
						S_USER_BASE* enemy_user = (S_USER_BASE*)enemy;
						if (enemy_user) enemy_pos = enemy_user->role.base.status.pos;
					}
						break;
					case N_MONSTER:
					case N_PET:
					case N_NPC:
					{
						S_ROBOT* enemy_robot = (S_ROBOT*)enemy;
						if (enemy_robot)
						{
							TS_Tools::posToGrid(&enemy_robot->bc.grid_pos, &enemy_pos, &map->leftpos);
							// ���ﲢ���ã���Ϊ���z����
							enemy_pos.z = atkpos.z;
						}
					}
						break;
					}

					// Ӧ�Գ�̼��ܣ�����㵽ֱ�ߵľ���ͽǶȻ�������������
					// ���㹹����һ���۽������Σ�ʹ����һ���ǳ������ķ�Χ�ڵĹ��ﶼ�ᱻ������
					// �����Ԥ��ɾ��һЩ�ṹ�ɶ۽������ε����
					f32 distance1 = TS_Tools::getDistance(&atkpos, &enemy_pos);
					if (distance1 > distance)
					{
						node = node->downnode;
						continue;
					}

					// ȷ���Ƕ�
					s32 angle = TS_Tools::computeAngle(&atkpos, &run->targetpos, &enemy_pos);
					if (angle > 70)
					{
						node = node->downnode;
						continue;
					}

					// �㵽ֱ�ߵľ���
					f32 PtLdistance = TS_Tools::getPointToLine_3D(&atkpos, &run->targetpos, &enemy_pos, value);
					if (PtLdistance > skillscript->radius)
					{
						node = node->downnode;
						continue;
					}

					// �����˺���buff
					TS_Tools::setSkillDamage(user, node->type, enemy, run, skillscript);
					TS_Tools::buff_skill_random(enemy, skillscript, node->type);
					damagecount -= 1;
					node = node->downnode;
				}
			}
		}
		return 0;
	}


	void S_USER_BASE::skill_end(S_TEMP_SKILL_RUN_BASE* run)
	{
		if (run->skill_id <= 0) return;
		auto skillscript = script::findScript_Skill(run->skill_id, run->skill_level);
		if (skillscript == nullptr) return;
		switch (skillscript->type)
		{
		case E_SKILL_USE_TYPE_DIR:
			onSkill_AtkDir(this, run, skillscript);
			break;
		case E_SKILL_USE_TYPE_SELF:
		case E_SKILL_USE_TYPE_TARGET:
		case E_SKILL_USE_TYPE_HALF:
			onSkill_AtkTarget(this, run, skillscript);
			break;
		}
		if (run->runingtime > 0) return;
		run->reset();
	}

	// ���������б���             ʱ��
	// 100msִ��һ��
	void S_USER_BASE::skill_running(const s32 value)
	{
		for (u32 i = 0; i < MAX_SKILL_COUNT; i++)
		{
			S_TEMP_SKILL_RUN_BASE* run = &this->tmp.temp_SkillRun.data[i];
			if (run->skill_id <= 0) continue;
			run->flytime -= value;
			if (run->flytime > 0) continue;
			run->flytime = 0;

			// ��֤ �ǲ��ǳ����˺�
			if (run->continued > 1)
			{
				run->runingtime -= value;
				run->temp_continuedtime += value;
				// �����˺����ʱ��
				if (run->temp_continuedtime < run->continued_waittime) continue;
				if (run->runingtime < 0) run->runingtime = 0;
				run->temp_continuedtime = 0;
			}
			else
			{
				run->runingtime = 0;
			}

			// ִ���˺��߼�
			skill_end(run);
		}
	}
}