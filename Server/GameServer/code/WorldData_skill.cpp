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

	// 开始技能	
	// 1.开始播放技能
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
			// 这里这个z如果要做高低差的话不可以
			// 不过既然是锁定了，那其实同一高度未必不可把？
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

		// 毕竟是有点延迟的，这里要考虑延迟	
		if (distance > skillscript->distance + 30)
		{
			LOG_MESSAGE("skill distance err... %d-%d [%d-%d-%d] [%d-%d-%d] line:%d \n",
				distance, skillscript->distance, this->role.base.status.pos.x,
				this->role.base.status.pos.y, this->role.base.status.pos.z,
				targetpos.x, targetpos.y, targetpos.z, __LINE__);
			return 3015;
		}

		// 查找一个空闲的技能
		auto run = this->tmp.temp_SkillRun.findFree();
		if (run == NULL) return 3016;
		
		run->targetpos = targetpos;
		run->lock_type = d->lock_type;
		run->lock_index = d->lock_index;
		run->skill_index = d->skillindex;		// d->skillindex是指的，其实这个run->skill_index和run->index用处不大

		run->skill_id = skillscript->id;
		run->skill_level = skillscript->level;
		run->damagecount = skillscript->damagecount;
		run->runingtime = skillscript->runingtime;
		run->continued = skillscript->continued;
		run->continued_waittime = skillscript->waittime;
		run->temp_continuedtime = 10000000;
		run->temp_calpos = this->role.base.status.pos;

		run->flytime = skillscript->flytime;	// 动作播放时间
		if(run->flytime > 10000)	// 远程攻击，需要飞行
		{
			run->flytime -= 10000;
			run->flytime += (distance * 0.5);	// 相隔300cm，就间隔300毫秒执行
		}

		// 重置CD时间
		this->tmp.temp_SkillPublicTime = global_gametime;
		this->tmp.temp_SkillColdTime[d->skillindex] = global_gametime;

		// 广播
		TS_Broadcast::bc_RoleSkill(this, run);

		// 冲刺的移动处理
		if (skillscript->id == 1006)
		{
			// 在世界内移动
			bool isupdata = false;
			
			this->moveWorld(&targetpos, isupdata);

			if (isupdata)
			{
				// 广播离开区域
				TS_Broadcast::bc_RoleMoveLeaveBig(&this->bc.edgeold, this);
			}
		}

		return 0;
	}

	s32 onSkill_AtkTarget(S_USER_BASE* user, S_TEMP_SKILL_RUN_BASE* run, script::SCRIPT_SKILL_BASE* skillscript)
	{
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return -1;

		// 定义一个可攻击的区域
		S_RECT_BASE rect;
		// 目标格子, 要转成大区域
		S_GRID_BASE target_grid;
		// 目标玩家索引位置
		s32 targetindex = -1;
		// 伤害玩家总数
		s32 damagecount = skillscript->damagecount;

		u32 distance = 0;

		void* enemy = NULL;
		S_USER_BASE* enemy_user = NULL;
		S_ROBOT*enemy_robot = NULL;
		// 是不是持续性技能
		if (run->continued <= 1)
		{
			// 有锁定目标
			if (run->lock_type != N_FREE)
			{
				// target_grid在此处就已经成功获取了
				enemy = TS_Tools::findSprite(&user->bc.edge, run->lock_type, run->lock_index, 
					user->node.layer, user->role.base.status.mapid, &target_grid);
				if (enemy == NULL) return -1;
				
				// 设置伤害
				// findSprite得时候找了一次enemy,这里又找一次，浪费性能
				// 应该来一个新的setSkillDamage函数，传入enemy进去转化更好
				TS_Tools::setSkillDamage(user, run->lock_type, enemy, run, skillscript);
				// 设置buff
				TS_Tools::buff_skill_random(enemy, skillscript, run->lock_type);

				targetindex = run->lock_index;
				damagecount -= 1;
				if (damagecount <= 0) return 0;
			}
		
			// 确定受击的位置，不管锁定与否，到这里都说明damagecount>=1，还能继续对其他目标造成伤害
			switch (run->lock_type)
			{
			case N_ROLE:
				enemy_user = (S_USER_BASE*)enemy;
				run->targetpos = enemy_user->role.base.status.pos;			// 更新目标坐标（只是为了防止目标移动）
				break;
			case N_MONSTER:
			case N_PET:
			case N_NPC:
				enemy_robot = (S_ROBOT*)enemy;
				TS_Tools::gridToPos(&enemy_robot->bc.grid_pos, &run->targetpos, &map->leftpos);// 更新目标坐标（只是为了防止目标移动）
				break;
			default:
				// 没有锁定目标的话会走这里来设置target_grid，有锁定目标的话target_grid早已经设置好
				TS_Tools::posToGrid(&target_grid, &run->targetpos, &map->leftpos);
				break;
			}

			// 确定攻击的小格子位置
			if (skillscript->type == E_SKILL_USE_TYPE_SELF)
			{
				target_grid = user->bc.grid_pos;
			}
		}
		else
		{
			// 持续性技能
			TS_Tools::posToGrid(&target_grid, &run->targetpos, &map->leftpos);
			if (skillscript->type == E_SKILL_USE_TYPE_SELF)
			{
				if (skillscript->isfollow != 0)
				{
					target_grid = user->bc.grid_pos;
				}
			}
		}

		// 上面确定了攻击的小格子，还要确定大格子，以确定哪些玩家会受到影响
		// 看看这个技能半径涵盖多少个小格子,向上取整
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

					// 判断距离
					bool isatk = TS_Tools::isInRadius(user, node->type, enemy, &run->targetpos, skillscript);
					if (!isatk)
					{
						node = node->downnode;
						continue;
					}

					// 设置伤害
					TS_Tools::setSkillDamage(user, node->type, enemy, run, skillscript);
					//设置buff
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
			// findSprite得时候找了一次enemy,这里又找一次，浪费性能
			// 应该来一个新的setSkillDamage函数，传入enemy进去转化更好
			TS_Tools::setSkillDamage(user, run->lock_type, enemy, run, skillscript);
			TS_Tools::buff_skill_random(enemy, skillscript, run->lock_type);
			damagecount -= 1;
			if (damagecount <= 0)return 0;
		}

		// 定义搜索玩家的区域
		S_RECT_BASE rect;
		S_VECTOR3 atkpos = user->role.base.status.pos;
		s32 distance = skillscript->distance;
		// 冲刺做特殊处理
		if (run->skill_id == 1006)
		{
			atkpos = run->temp_calpos;
			distance = TS_Tools::getDistance(&atkpos, &run->targetpos);
		}

		s32 glen = ceil(skillscript->radius / C_WORLDMAP_ONE_GRID);
		if (!map->Get_Redius_View(&user->bc.grid_pos, glen, &rect)) return -3;

		// 计算点到直线的被除数
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
					// 确定节点的位置
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
							// 这里并不好，因为这个z坐标
							enemy_pos.z = atkpos.z;
						}
					}
						break;
					}

					// 应对冲刺技能，下面点到直线的距离和角度会出现意外情况：
					// 三点构成了一个钝角三角形，使得在一个非常狭长的范围内的怪物都会被攻击到
					// 这里就预先删掉一些会构成钝角三角形的情况
					f32 distance1 = TS_Tools::getDistance(&atkpos, &enemy_pos);
					if (distance1 > distance)
					{
						node = node->downnode;
						continue;
					}

					// 确定角度
					s32 angle = TS_Tools::computeAngle(&atkpos, &run->targetpos, &enemy_pos);
					if (angle > 70)
					{
						node = node->downnode;
						continue;
					}

					// 点到直线的距离
					f32 PtLdistance = TS_Tools::getPointToLine_3D(&atkpos, &run->targetpos, &enemy_pos, value);
					if (PtLdistance > skillscript->radius)
					{
						node = node->downnode;
						continue;
					}

					// 设置伤害、buff
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

	// 技能运行列表检查             时间
	// 100ms执行一次
	void S_USER_BASE::skill_running(const s32 value)
	{
		for (u32 i = 0; i < MAX_SKILL_COUNT; i++)
		{
			S_TEMP_SKILL_RUN_BASE* run = &this->tmp.temp_SkillRun.data[i];
			if (run->skill_id <= 0) continue;
			run->flytime -= value;
			if (run->flytime > 0) continue;
			run->flytime = 0;

			// 验证 是不是持续伤害
			if (run->continued > 1)
			{
				run->runingtime -= value;
				run->temp_continuedtime += value;
				// 两次伤害间隔时间
				if (run->temp_continuedtime < run->continued_waittime) continue;
				if (run->runingtime < 0) run->runingtime = 0;
				run->temp_continuedtime = 0;
			}
			else
			{
				run->runingtime = 0;
			}

			// 执行伤害逻辑
			skill_end(run);
		}
	}
}