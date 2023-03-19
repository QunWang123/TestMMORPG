#include "RobotData.h"
#include "WorldTools.h"
#include "AppGlobal.h"
#include "WorldBC.h"

namespace app
{
	// 玩家攻击怪物，会引起怪物思考
	void S_ROBOT::event_RoleAtk(const s32 index)
	{
		if (data.ai.base.lock_type == N_FREE)
		{
			event_SetLock(N_ROLE, index);
		}
		else
		{
			// 有锁定目标了，但有30%的概率转火
			u32 atk = TS_Tools::RandomRange(0, 1000);
			if (atk < 300) this->event_SetLock(N_ROLE, index);
		}

		switch (this->data.ai.think)
		{
		case AI_MOVE_RAND:
		case AI_STATE_UPDATE:
			// 设置为空闲状态，准备开始执行新的逻辑
			this->data.ai.think = AI_FREE;
			break;
		}
		this->event_AI(index);
	}

	// 由玩家在worddata中调用
	// robotmanager也会调用
	// index==-1为robotmanager调用
	// index是指调用到该函数的玩家的下标
	void S_ROBOT::event_AI(const s32 index)
	{
		if (data.status.state == E_SPRITE_STATE_FREE) return;
		// index == -1是机器人管理器的触发
		if (index == -1)
		{
			s32  ftime = global_gametime - data.ai.limit.time_think;
			if (ftime < 100) return;
			data.ai.limit.time_think = global_gametime;
		}

		//运行技能和BUFF
		s32 value = global_gametime - this->tmp.temp_RunningTime;
		if (value >= 100)
		{
 			skill_running(value);//技能运行
 			TS_Tools::buff_runing(this, N_MONSTER, value);//buff运行
			this->tmp.temp_RunningTime = global_gametime;
		}
		else if (value <= 0)
		{
			this->tmp.temp_RunningTime = global_gametime;
		}

		//思考
		// 返回false的情况：
		// 1.已经死透了，没法复活
		// 2.这个机器人是个宠物
		// 3.这个机器人正在巡逻/攻击	
		if (event_Think(index) == false) return;

		//根据上面的思考来执行怪物的行为
		switch (data.ai.think)
		{
		case AI_STATE_UPDATE:Think_StateUpdate(); break;//1、状态更新
 		case AI_MOVE_RAND:   Think_MoveRand();    break;//2、随机移动
 		case AI_MOVE_ATK:    Think_MoveAtk();     break;//3、移向攻击目标
 		case AI_MOVE_RETURN: Think_MoveReturn();  break;//4、返回出生点
 		case AI_RELIVE_SET:  Think_SetReborn();   break;//5、复活开始
 		case AI_RELIVE_DO:   Think_Reborn();      break;//6、复活结束
// 		case AI_CLEAR:       Think_Clear();       break;//7、清理
		}
	}

	bool S_ROBOT::event_Think(const s32 index)
	{
		if (tmp.script == nullptr) return false;
		if (isLive() == false && (data.ai.think != AI_RELIVE_SET && data.ai.think != AI_RELIVE_DO))     return false;
		//执行怪物的行为
		switch (data.ai.think)
		{
		case AI_MOVE_RETURN:
		case AI_RELIVE_SET:
		case AI_RELIVE_DO:
		case AI_STATE_UPDATE:
		case AI_CLEAR:
			return true;
		}


		if (node.type == N_PET)       return false;
		if (data.ai.think != AI_FREE) return false;

		if (index < 0)
		{
			//unlock
			if (data.ai.base.lock_type > 0)
			{
				void* enemy = TS_Tools::findSprite(&this->bc.edge,
					this->data.ai.base.lock_type,
					this->data.ai.base.lock_id,
					this->node.layer,
					this->data.status.mapid);
				if (enemy == NULL)
				{
					setLock(N_FREE, -1);
				}
			}
			return true;
		}


		//1、锁定目标 主动攻击怪物
		if (data.ai.base.lock_type == N_FREE && tmp.script->isatk > 0)
		{
			event_SetLock(N_ROLE, index);
		}
		//2、有锁定类型vs攻击目标
		if (data.ai.base.lock_type > N_FREE)
		{
			bool isUseSkill = skill_start();
			// this->data.ai.think = AI_MOVE_ATK;
			if(isUseSkill) return true;
		}
		//3、巡逻-随机移动
		if (tmp.script->range > 0)
		{
			if (tmp.temp_buffRun.isMove() && data.ai.isTimeMoveRND())
			{
				data.ai.think = AI_MOVE_RAND;
				return true;
			}
		}
		//4、状态恢复
		if (this->data.ai.isTimeRenew())
		{
			this->data.ai.think = AI_STATE_UPDATE;
			return true;
		}

		return true;
	}


	void S_ROBOT::event_SetLock(const u8 kind, const u32 index)
	{
		if (!isLockDistance(kind, index)) return;
		setLock(kind, index);
	}


}


