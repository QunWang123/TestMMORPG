#include "RobotData.h"
#include "AppGlobal.h"
#include "WorldTools.h"
#include "RobotManager.h"
#include "WorldBC.h"

namespace app
{

	void S_ROBOT_AI::randTime(u32 id)
	{
		u32 value = time(NULL) + id * 358;
		srand(value);

		limit.move_random_min = rand() % 2000 + 3000;
		limit.move_random_max = rand() % 5000 + 8000;
		limit.move_atk_min = 2000;
		limit.move_atk_max = 2000;
	}
	// 随机移动是否结束
	bool S_ROBOT_AI::isTimeMoveRND()
	{
		if (limit.time_move > global_gametime)
		{
			// 此时尚不能随机移动
			return false;
		}
		else if (limit.time_move + limit.move_atk_max < global_gametime)
		{
			// 说明limit.time_move尚未被设置，进行初始化
			// 这里只有limit.time_move尚未设置的时候才会走
			limit.time_move = global_gametime + TS_Tools::RandomRange(limit.move_random_min, limit.move_random_max);
			return false;
		}
		else
		{
			limit.time_renew = global_gametime + limit.move_random_min;
			limit.time_move = global_gametime + TS_Tools::RandomRange(limit.move_random_min, limit.move_random_max);
			return true;
		}
		return false;
	}

	bool S_ROBOT_AI::isTimeMoveATK()
	{
		s32 value = global_gametime - limit.time_move_atk;
		if (value < 0) return false;

		// limit.time_move_atk = global_gametime + TS_Tools::RandomRange(1300, 1800);
		limit.time_move_atk = global_gametime + 100;
		return true;
	}
	// 是否可以执行状态恢复
	bool S_ROBOT_AI::isTimeRenew()
	{
		s32 value = global_gametime - limit.time_renew;
		if (value < 0) return false;

		limit.time_renew = global_gametime + TS_Tools::RandomRange(limit.move_random_min, limit.move_random_max);
		return true;
	}

	bool S_ROBOT_AI::isTimeMoveReturn()
	{
		s32 value = global_gametime - limit.time_tempreturn;
		if (value < 0) return false;

		limit.time_tempreturn = global_gametime + 400;
		return true;
	}

	bool S_ROBOT::isT()
	{
		return data.status.id > 0;
	}

	bool S_ROBOT::isLive()
	{
		return data.status.hp > 0;
	}



	u8 S_ROBOT::getSpriteCollideType()
	{
		switch (node.type)
		{
		case N_MONSTER:
			return E_SPRITE_COLLIDE_MONSTER;
		case N_NPC:
			return E_SPRITE_COLLIDE_NPC;
		case N_PET:
			return E_SPRITE_COLLIDE_PET;
		default:
			break;
		}
		return 0;
	}


	void S_ROBOT::setLock(u8 kind, u32 index)
	{
		data.ai.base.lock_type = kind;
		data.ai.base.lock_id = index;
	}


	bool S_ROBOT::isLockDistance(const u8 type, const u32 index)
	{
		if (tmp.script == nullptr) return false;

		S_GRID_BASE grid;
		void* enemy = TS_Tools::findSprite(&this->bc.edge, type, index, node.layer, data.status.mapid, &grid);
		if (enemy == NULL) return false;
		s32 distance = TS_Tools::getDistance(&grid, &this->bc.grid_pos);

		if (distance <= tmp.script->alert) return true;
		return false;
	}

	void S_ROBOT::resetDead()
	{
		if (data.status.isreborn)
		{
			this->data.ai.think = AI_RELIVE_SET;
			this->data.status.state = E_SPRITE_STATE_DEAD;
			// 怪物死亡刷新时间
			if (tmp.script != nullptr)
			{
				data.ai.limit.time_relive = global_gametime + tmp.script->time_death;
			}
			else
			{
				data.ai.limit.time_relive = global_gametime + 5000;
			}
		}
		else
		{
			this->data.ai.think = AI_CLEAR;
			this->data.status.state = E_SPRITE_STATE_DEAD;
		}
		__RobotManager->count -= 1;
		if (__RobotManager->count <= 0) __RobotManager->count = 0;
	}

	void S_ROBOT::setDead()
	{
		resetDead();
	}

	// 回复活点
	void S_ROBOT::setMoveReturn()
	{
		if (tmp.script == nullptr) return;
		if (tmp.script->range > 0)
		{
			this->data.ai.think = AI_MOVE_RETURN;
			this->data.ai.limit.time_movereturn = global_gametime + 1000 * 8;
		}
		else
		{
			this->data.ai.think = AI_FREE;
		}
		this->data.ai.base.skill_index = -1;
		this->data.ai.base.skill_level = 0;
		setLock(N_FREE, -1);
		this->data.status.hp = tmp.script->maxhp;
		this->data.status.mp = tmp.script->maxmp;
		TS_Broadcast::bc_RobotState(this, CMD_8100);
	}
}