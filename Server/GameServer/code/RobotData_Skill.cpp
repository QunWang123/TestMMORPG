#include "RobotData.h"
#include "WorldTools.h"
#include "WorldMap.h"
#include "AppGlobal.h"
#include "WorldBC.h"

namespace app
{
	// 不在公共CD时间就返回true
	bool S_ROBOT::skill_IsPublicTime()
	{
		s32 value = global_gametime - this->tmp.temp_SkillPublicTime;
		if (value < MAX_SKILL_PUBLICTIME)
		{
			return false;
		}
		return true;
	}


	bool S_ROBOT::skill_IsColdTime(const u8 skill_index, const u32 time)
	{
		if (skill_index >= MAX_SKILL_COUNT)return false;
		s32 value = global_gametime - this->tmp.temp_SkillColdTime[skill_index];
		if (value < time)
		{
			return false;
		}
		return true;
	}

	bool S_ROBOT::skill_start()
	{
		if (tmp.script == nullptr) return false;
		if (tmp.script->skill_ids.size() <= 0) return false;

		u32 skill_index = TS_Tools::Random(tmp.script->skill_ids.size());
		u32 skill_level = 1;
		u32 skill_id = tmp.script->skill_ids[skill_index];

		// 获取技能脚本
		script::SCRIPT_SKILL_BASE* skillscript = script::findScript_Skill(skill_id, skill_level);
		if (skillscript == nullptr)
		{
			LOG_MESSAGE("robot skill script err... %d/%d\n", skill_id, data.ai.base.skill_level);
			setLock(N_FREE, -1);
			data.ai.think = AI_FREE;
			return false;
		}

		S_WORLD_MAP* map = __World->getMap(data.status.mapid);
		if (map == nullptr)
		{
			LOG_MESSAGE("robot skill map err... %d\n", data.status.mapid);
			setLock(N_FREE, -1);
			data.ai.think = AI_FREE;
			return false;
		}
		// 机器人死亡
		if (!this->isLive())
		{
			LOG_MESSAGE("robot skill err...robot dead %d\n", node.index);
			setLock(N_FREE, -1);
			data.ai.think = AI_FREE;
			return true;
		}
		// 验证技能锁定目标
		S_GRID_BASE lock_grid;
		void* lock_target = TS_Tools::findSprite(&this->bc.edge,
			this->data.ai.base.lock_type,
			this->data.ai.base.lock_id,
			this->node.layer,
			this->data.status.mapid,
			&lock_grid);
		if (lock_target == NULL)
		{
			LOG_MESSAGE("robot skill err...locktarget null %d\n", this->data.ai.base.lock_id);
			setLock(N_FREE, -1);
			data.ai.think = AI_FREE;
			return false;
		}
		// 验证目标是否死亡
		if (this->data.ai.base.lock_type == N_ROLE)
		{
			S_USER_BASE* lock_user = (S_USER_BASE*)lock_target;
			if (lock_user && !lock_user->isLive())
			{
				setLock(N_FREE, -1);
				data.ai.think = AI_FREE;
				return false;
			}
		}
		else
		{
			S_ROBOT* lock_robot = (S_ROBOT*)lock_target;
			if (lock_robot && !lock_robot->isLive())
			{
				setLock(N_FREE, -1);
				data.ai.think = AI_FREE;
				return false;
			}
		}

		// 状态恢复
		if (this->data.ai.isTimeRenew())
		{
			data.ai.think = AI_STATE_UPDATE;
			return true;
		}
		// 验证敌人是否出了怪物的移动区域
		bool isout = this->data.status.move_edge.outEdge(&lock_grid);
		if (isout)
		{
			setMoveReturn();
			return true;
		}
		// 验证技能范围
		S_VECTOR3 lock_pos;
		S_VECTOR3 self_pos;
		TS_Tools::gridToPos(&lock_grid, &lock_pos, &map->leftpos);
		TS_Tools::gridToPos(&this->bc.grid_pos, &self_pos, &map->leftpos);
		u32 distance = TS_Tools::getDistance(&lock_pos, &self_pos);
		if (distance > skillscript->distance)
		{
			this->data.ai.think = AI_MOVE_ATK;
			return true;
		}
		// 验证CD时间
		bool isTime = this->skill_IsPublicTime();
		if (!isTime)
		{
			return true;
		}
		isTime = this->skill_IsColdTime(skill_index, skillscript->coldtime);
		if (!isTime)
		{
			return true;
		}
		// 验证是否中了BUFF不能用技能
		if (!this->tmp.temp_buffRun.isUseSkill())
		{
			LOG_MESSAGE("robot skill err...buff %d\n", node.index);
			return true;
		}

		auto run = this->tmp.temp_SkillRun.findFree();
		if (run == NULL)
		{
			setLock(N_FREE, -1);
			data.ai.think = AI_FREE;
			return true;
		}
		// 设置技能
		this->data.ai.base.skill_index = skill_index;
		this->data.ai.base.skill_level = 1;

		run->targetpos = lock_pos;
		run->lock_type = this->data.ai.base.lock_type;
		run->lock_index = this->data.ai.base.lock_id;
		run->skill_index = skill_index;
		run->skill_id = skillscript->id;
		run->skill_level = skillscript->level;
		run->damagecount = skillscript->damagecount;
		run->runingtime = skillscript->runingtime;
		run->continued = skillscript->continued;
		run->continued_waittime = skillscript->waittime;
		run->temp_continuedtime = 1000000;

		run->flytime = skillscript->flytime;
		if (run->flytime >= 10000)	// 是远程攻击
		{
			run->flytime -= 10000;
			run->flytime += (distance * 0.5);
		}
		// 重置CD时间
		this->tmp.temp_SkillPublicTime = global_gametime;
		this->tmp.temp_SkillColdTime[skill_index] = global_gametime;
		TS_Broadcast::bc_RobotState(this, CMD_8500);
		return true;
	}

	//使用技能攻击目标
	s32  onSkill_AtkTarget(S_ROBOT* robot, S_TEMP_SKILL_RUN_BASE * run, script::SCRIPT_SKILL_BASE * skillscript)
	{
		S_WORLD_MAP* map = __World->getMap(robot->data.status.mapid);
		if (map == nullptr) return -1;

		//定义一个区域
		S_RECT_BASE  rect;
		//目标格子 转换成大的区域
		S_GRID_BASE  target_grid;
		//目标玩家索引
		s32 targetindex = -1;
		//伤害玩家总数
		s32 damagecount = run->damagecount;
		u32 distance = 0;

		//是不是持续性技能
		if (run->continued <= 1)
		{
			//根据类型 查找精灵
			void* enemy = TS_Tools::findSprite(&robot->bc.edge, run->lock_type, run->lock_index, robot->node.layer, 
												robot->data.status.mapid, &target_grid);
			if (enemy == NULL) return -1;

			//设置伤害
			TS_Tools::setSkillDamage(robot, run->lock_type, enemy, run, skillscript);
			//设置buff
			if (enemy) TS_Tools::buff_skill_random(enemy, skillscript, run->lock_type);

			S_USER_BASE* enemy_user = NULL;
			S_ROBOT* enemy_robot = NULL;
			switch (run->lock_type)
			{
			case N_ROLE:
				enemy_user = (S_USER_BASE*)enemy;
				if (enemy_user) run->targetpos = enemy_user->role.base.status.pos; //// 更新目标坐标
				break;
			case N_MONSTER:
			case N_PET:
			case N_NPC:
				enemy_robot = (S_ROBOT*)enemy;
				if (enemy_robot) TS_Tools::gridToPos(&enemy_robot->bc.grid_pos, &run->targetpos, &map->leftpos);//// 更新目标坐标
				break;
			default:
				return -2;
			}

			targetindex = run->lock_index;
			damagecount -= 1;
			if (damagecount <= 0) return  0;

			if (skillscript->type == E_SKILL_USE_TYPE_SELF)
			{
				target_grid = robot->bc.grid_pos;
			}
		}
		else
		{
			if (run->lock_type == N_FREE) return -3;

			TS_Tools::posToGrid(&target_grid, &run->targetpos, &map->leftpos);
			if (skillscript->type == E_SKILL_USE_TYPE_SELF)
			{
				if (skillscript->isfollow != 0)
				{
					target_grid = robot->bc.grid_pos;
				}
			}
		}

		//目标格子转为区域 搜索玩家范围的区域  向上取整 1.2 = 2
		s32 glen = ceil(skillscript->radius / C_WORLDMAP_ONE_GRID);
		bool isview = map->Get_Redius_View(&target_grid, glen, &rect);
		if (isview == false) return -3;

		LOG_MESSAGE("robot skill rect...%d,%d-%d,%d-%d \n", robot->node.index, rect.left, rect.top, rect.right, rect.bottom);

		//开始计算伤害
		for (u32 row = rect.top; row <= rect.bottom; row++)
		{
			for (u32 col = rect.left; col <= rect.right; col++)
			{
				if (damagecount <= 0) break;
				if (!map->IsRect(row, col)) continue;

				S_WORLD_NODE * node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					if (damagecount <= 0) break;
					if (node->index == targetindex ||
						((node->type == N_MONSTER || node->type == N_PET || node->type == N_NPC) && node->index == robot->node.index)
						|| node->layer != robot->node.layer)
					{
						node = node->downnode;
						continue;
					}

					//根据类型 查找精灵
					void* enemy = TS_Tools::findSprite(&robot->bc.edge, node->type, node->index, node->layer, robot->data.status.mapid);
					if (enemy == NULL)
					{
						node = node->downnode;
						continue;
					}

					//验证敌人是不是在攻击距离内？
					bool isatk = TS_Tools::isInRadius(robot, node->type, enemy, &run->targetpos, skillscript);
					if (isatk == false)
					{
						node = node->downnode;
						continue;
					}

					//设置伤害
					TS_Tools::setSkillDamage(robot, node->type, enemy, run, skillscript);
					//设置buff
					if (enemy) TS_Tools::buff_skill_random(enemy, skillscript, node->type);

					damagecount--;
					node = node->downnode;
				}
			}
		}
		return 0;
	}

	//2、打方向的技能
	s32 onSkill_AtkDir(S_ROBOT* robot, S_TEMP_SKILL_RUN_BASE * run, script::SCRIPT_SKILL_BASE * skillscript)
	{
		S_WORLD_MAP* map = __World->getMap(robot->data.status.mapid);
		if (map == nullptr) return -1;

		s32 damagecount = skillscript->damagecount;
		s32 targetindex = run->lock_index;

		//根据类型 查找精灵
		void* enemy = TS_Tools::findSprite(&robot->bc.edge, run->lock_type, run->lock_index, robot->node.layer, robot->data.status.mapid);
		if (enemy == NULL) return -2;
		// 怪物一定是有了锁定目标才会放技能
		//设置伤害
		TS_Tools::setSkillDamage(robot, run->lock_type, enemy, run, skillscript);
		//设置buff
		if (enemy) TS_Tools::buff_skill_random(enemy, skillscript, run->lock_type);

		damagecount -= 1;
		if (damagecount <= 0) return 0;


		// 定义搜索玩家的区域
		S_RECT_BASE  rect;
		// 计算攻击距离
		s32 distance = skillscript->distance;

		// 目标格子转为区域 搜索玩家范围的区域  向上取整 1.2 = 2
		s32 glen = ceil(distance / C_WORLDMAP_ONE_GRID);
		bool isview = map->Get_Redius_View(&robot->bc.grid_pos, glen, &rect);
		if (isview == false) return -3;

		//计算被除数
		S_VECTOR3 self_pos;
		TS_Tools::gridToPos(&robot->bc.grid_pos, &self_pos, &map->leftpos);
		f32 value = TS_Tools::getPointToLine(&self_pos, &run->targetpos);


		//设置BUFF
		S_USER_BASE* enemy_user = NULL;
		S_ROBOT* enemy_robot = NULL;

		//开始计算伤害
		for (u32 row = rect.top; row <= rect.bottom; row++)
			for (u32 col = rect.left; col <= rect.right; col++)
			{
				if (damagecount <= 0) break;
				if (!map->IsRect(row, col)) continue;

				//获取九宫格根节点
				S_WORLD_NODE * node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					if (damagecount <= 0) break;
					if (node->index == targetindex ||
						((node->type == N_MONSTER || node->type == N_PET || node->type == N_NPC) && node->index == robot->node.index)
						|| node->layer != robot->node.layer)
					{
						node = node->downnode;
						continue;
					}

					//根据类型 查找精灵
					enemy = TS_Tools::findSprite(&robot->bc.edge, node->type, node->index, node->layer, robot->data.status.mapid);
					if (enemy == NULL)
					{
						node = node->downnode;
						continue;
					}

					S_USER_BASE* enemy_user = NULL;
					S_ROBOT* enemy_robot = NULL;
					S_VECTOR3 enemy_pos;
					switch (node->type)
					{
					case N_ROLE:
						enemy_user = (S_USER_BASE*)enemy;
						enemy_pos = enemy_user->role.base.status.pos;
						break;
					case N_MONSTER:
					case N_PET:
					case N_NPC:
						enemy_robot = (S_ROBOT*)enemy;
						TS_Tools::gridToPos(&enemy_robot->bc.grid_pos, &enemy_pos, &map->leftpos);
						break;
					}
					f32 distance1 = TS_Tools::getDistance(&self_pos, &enemy_pos);
					if (distance1 > distance)
					{
						node = node->downnode;
						continue;
					}
					//1 计算角度
					//1自己的坐标 2目标位置 3需要检查的玩家的坐标
					s32 angle = TS_Tools::computeAngle(&self_pos, &run->targetpos, &enemy_pos);
					if (angle > 70)
					{
						node = node->downnode;
						continue;
					}
					// 计算点到直线的距离
					f32 distance2 = TS_Tools::getPointToLine(&self_pos, &run->targetpos, &enemy_pos, value);
					if (distance2 > skillscript->radius)
					{
						node = node->downnode;
						continue;
					}
					//设置伤害
					TS_Tools::setSkillDamage(robot, node->type, enemy, run, skillscript);
					//设置buff
					if (enemy) TS_Tools::buff_skill_random(enemy, skillscript, node->type);

					node = node->downnode;
				}
			}
		return 0;

	}

	void S_ROBOT::skill_running(const s32 value)
	{
		if (this->data.status.state == E_SPRITE_STATE_FREE) return;
		if (this->isLive() == false) return;
		if (this->tmp.script == nullptr) return;

		u32 length = this->tmp.script->skill_ids.size();
		if (length >= MAX_SKILL_COUNT) return;
		for (u32 i = 0; i < length; i++)
		{
			S_TEMP_SKILL_RUN_BASE* skill_run = &this->tmp.temp_SkillRun.data[i];
			if (skill_run->skill_id <= 0)continue;

			skill_run->flytime -= value;
			if (skill_run->flytime > 0) continue;
			skill_run->flytime = 0;
			// 判断是不是持续技能
			if (skill_run->continued > 1)
			{
				skill_run->runingtime -= value;
				skill_run->temp_continuedtime += value;
				if (skill_run->temp_continuedtime < skill_run->continued_waittime) continue;
				if (skill_run->runingtime < 0) skill_run->runingtime = 0;
				skill_run->temp_continuedtime = 0;
			}
			else
			{
				skill_run->runingtime = 0;
			}
			skill_end(skill_run);
		}
	}

	void S_ROBOT::skill_end(S_TEMP_SKILL_RUN_BASE* run)
	{
		if (run->skill_id <= 0)return;
		auto skillscript = script::findScript_Skill(run->skill_id, run->skill_level);
		if (skillscript == nullptr)return;
		switch (skillscript->type)
		{
		case E_SKILL_USE_TYPE_DIR://打方向的技能
			//定义伤害的API
			onSkill_AtkDir(this, run, skillscript);
			break;
		case E_SKILL_USE_TYPE_SELF:
		case E_SKILL_USE_TYPE_TARGET:
		case E_SKILL_USE_TYPE_HALF:
			//定义伤害的API
			onSkill_AtkTarget(this, run, skillscript);
			break;
		}
		if (run->runingtime > 0) return;
		run->reset();
	}
}

