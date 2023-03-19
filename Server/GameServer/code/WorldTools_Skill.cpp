#include "WorldTools.h"
#include "WorldMap.h"
#include "UserManager.h"
#include "WorldBC.h"
#include "RobotManager.h"


namespace app
{
	void TS_Tools::setSkillDamage(S_USER_BASE* user, u8 type, void* enemy, S_TEMP_SKILL_RUN_BASE* run, void* sk)
	{
		if (user == NULL || !user->isLive() || sk == NULL || enemy == NULL || run == NULL) return;
		
		script::SCRIPT_SKILL_BASE* skillscript = (script::SCRIPT_SKILL_BASE*)sk;
		s32 skill_hp = skillscript->damagehp;

		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == nullptr) return;

		S_USER_BASE* enemy_user = nullptr;
		S_ROBOT* enemy_robot = nullptr;

		switch (type)
		{
		case N_ROLE:
			enemy_user = (S_USER_BASE*)enemy;
			if (!enemy_user->isLive()) return;
			break;
		case N_MONSTER:
		case N_PET:
		case N_NPC:
			enemy_robot = (S_ROBOT*)enemy;
			if (!enemy_robot->isLive()) return;
			break;
		default:
			return;
		}
		if (enemy_user != NULL)
		{
			if (user->node.index == enemy_user->node.index) return;//�ж��ǲ����Լ�
			if (user->isAtkRole(enemy_user) == false)       return;//�ǲ��ǿ��Թ�����

			//���㹥����ʽ
			s32 atk = user->atk.total.p_atk - enemy_user->atk.total.p_defend + skill_hp;
			enemy_user->role.base.life.hp -= atk;
			LOG_MESSAGE("hp...%d-%d-%d\n", enemy_user->node.index, enemy_user->role.base.life.hp, atk);

			TS_Broadcast::bc_RoleSkillHP(user, type, enemy_user->node.index, atk, enemy_user->role.base.life.hp);
			if (enemy_user->role.base.life.hp <= 0)
			{
				// ����״̬
				enemy_user->roleState_SetDead();
				// ����
				TS_Broadcast::bc_RoleDead(enemy_user);
			}
			return;
		}
		//2����ҹ�������
		if (enemy_robot != NULL)
		{
			// ��ҹ������� ��������˼����
			enemy_robot->event_RoleAtk(user->node.index);

			// ���㹥��ֵ�����������е���֣����нű������def������data�����ħ�������������Ӧ������ħ���������
			s32 atk = user->atk.total.p_atk + 100 - 10 + skill_hp;
			if (enemy_robot->tmp.script)  atk -= enemy_robot->tmp.script->def;
			if (atk <= 0)atk = 1;
			enemy_robot->data.status.hp -= atk;
			//�㲥XX����յ��˺�
			TS_Broadcast::bc_RoleSkillHP(user, type, enemy_robot->node.index, atk, enemy_robot->data.status.hp);
			//�������������¾��飬���µ���
			if (enemy_robot->data.status.hp <= 0)
			{
				// user->mainTask_Update(enemy_robot->data.status.id);
				//���þ���
				if (enemy_robot->tmp.script) user->atk_UpdateExp(enemy_robot->tmp.script->exp);
				//����״̬
				enemy_robot->data.status.hp = 0;
				enemy_robot->setDead();
				//�㲥������Ϣ
				TS_Broadcast::bc_RobotState(enemy_robot, CMD_8300);

				//�������
				if (enemy_robot->tmp.script != NULL)
				{
					u32 size = enemy_robot->tmp.script->drop_ids.size();
					for (int i = 0; i < size; i++)
					{
						//�������ðٷְٵ���
						script::SCRIPT_DROP_ID* drop = enemy_robot->tmp.script->drop_ids[i];
						s32 value = 0;// TS_Tools::Random(10000);
						if (value < drop->probability)
						{
							//1 ���߰��ڵ�����Ʒ����
							S_ROLE_PROP prop;
							prop.reset();
							//2 ��������(ջ�п���һ���ڴ棬��������ڴ�Ļ����������ú�)
							bool isok = TS_Tools::createProp(drop->id, EPF_MONSTER, 1, -1, "robot", &prop);
							if (isok == false) continue;

							//3 ���õ��䣬���ϱ߽��õ�ջ���ڴ��һ�ж�������world_drop��datas����
							prop.base.id = drop->id;
							map->Drop.createRobotDrop(map->mapID, &enemy_robot->bc.grid_pos, user->node.index, 0, user->node.layer, &prop);
							// break;
						}
					}
 				}
			}
			return;
		}
	}


	void TS_Tools::setSkillDamage(void* _robot, u8 type, void* enemy, S_TEMP_SKILL_RUN_BASE* run, void* sk)
	{
		if (_robot == NULL || sk == NULL || enemy == NULL || run == NULL) return;
		S_ROBOT* robot = (S_ROBOT*)_robot;
		if(robot == NULL || !robot->isLive()) return;

		script::SCRIPT_SKILL_BASE* skillscript = (script::SCRIPT_SKILL_BASE*)sk;
		s32 skill_hp = skillscript->damagehp;

		S_WORLD_MAP* map = __World->getMap(robot->data.status.mapid);
		if (map == nullptr) return;

		S_USER_BASE* enemy_user = nullptr;
		S_ROBOT* enemy_robot = nullptr;

		switch (type)
		{
		case N_ROLE:
			enemy_user = (S_USER_BASE*)enemy;
			if (!enemy_user->isLive()) return;
			break;
		case N_MONSTER:
		case N_PET:
		case N_NPC:
			enemy_robot = (S_ROBOT*)enemy;
			if (!enemy_robot->isLive()) return;
			break;
		default:
			return;
		}

		if (enemy_user != NULL)
		{
			// if (user->node.index == enemy_user->node.index) return;//�ж��ǲ����Լ�
			if (enemy_user->isAtkRole(enemy_user) == false)       return;//�ǲ��ǿ��Թ�����

			//���㹥����ʽ
			s32 atk = 10;
			if(robot->tmp.script) atk = robot->tmp.script->atk - enemy_user->atk.total.p_defend + skill_hp;
			atk = max(10, atk);
			enemy_user->role.base.life.hp -= atk;
			LOG_MESSAGE("hp...%d-%d-%d\n", enemy_user->node.index, enemy_user->role.base.life.hp, atk);

			TS_Broadcast::bc_RobotSkillHP(robot, type, enemy_user->node.index, atk, enemy_user->role.base.life.hp);
			if (enemy_user->role.base.life.hp <= 0)
			{
				// ����״̬
				enemy_user->roleState_SetDead();
				// ����
				TS_Broadcast::bc_RoleDead(enemy_user);
			}
			return;
		}
		//2�����﹥������
		if (enemy_robot != NULL)
		{
			//�������˺�1-100��
			s32 atk = 10;
			if (robot->tmp.script && enemy_robot->tmp.script)  atk = skill_hp + robot->tmp.script->atk - enemy_robot->tmp.script->def;

			enemy_robot->data.status.hp -= atk;
			//LOG_MSG("enemy_robot hp...%d//%d//%d \n", enemy_robot->node.index, enemy_robot->data.status.hp, atk);

			//�㲥XX����յ��˺�
			TS_Broadcast::bc_RobotSkillHP(robot, type, enemy_robot->node.index, atk, enemy_robot->data.status.hp);
			//�������
			if (enemy_robot->data.status.hp <= 0)
			{
				enemy_robot->data.status.hp = 0;
				//����״̬
				enemy_robot->setDead();
				//�㲥������Ϣ
				TS_Broadcast::bc_RobotState(enemy_robot, CMD_8300);
			}
			return;
		}
	}
}

