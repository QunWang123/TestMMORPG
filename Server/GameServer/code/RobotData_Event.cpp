#include "RobotData.h"
#include "WorldTools.h"
#include "AppGlobal.h"
#include "WorldBC.h"

namespace app
{
	// ��ҹ���������������˼��
	void S_ROBOT::event_RoleAtk(const s32 index)
	{
		if (data.ai.base.lock_type == N_FREE)
		{
			event_SetLock(N_ROLE, index);
		}
		else
		{
			// ������Ŀ���ˣ�����30%�ĸ���ת��
			u32 atk = TS_Tools::RandomRange(0, 1000);
			if (atk < 300) this->event_SetLock(N_ROLE, index);
		}

		switch (this->data.ai.think)
		{
		case AI_MOVE_RAND:
		case AI_STATE_UPDATE:
			// ����Ϊ����״̬��׼����ʼִ���µ��߼�
			this->data.ai.think = AI_FREE;
			break;
		}
		this->event_AI(index);
	}

	// �������worddata�е���
	// robotmanagerҲ�����
	// index==-1Ϊrobotmanager����
	// index��ָ���õ��ú�������ҵ��±�
	void S_ROBOT::event_AI(const s32 index)
	{
		if (data.status.state == E_SPRITE_STATE_FREE) return;
		// index == -1�ǻ����˹������Ĵ���
		if (index == -1)
		{
			s32  ftime = global_gametime - data.ai.limit.time_think;
			if (ftime < 100) return;
			data.ai.limit.time_think = global_gametime;
		}

		//���м��ܺ�BUFF
		s32 value = global_gametime - this->tmp.temp_RunningTime;
		if (value >= 100)
		{
 			skill_running(value);//��������
 			TS_Tools::buff_runing(this, N_MONSTER, value);//buff����
			this->tmp.temp_RunningTime = global_gametime;
		}
		else if (value <= 0)
		{
			this->tmp.temp_RunningTime = global_gametime;
		}

		//˼��
		// ����false�������
		// 1.�Ѿ���͸�ˣ�û������
		// 2.����������Ǹ�����
		// 3.�������������Ѳ��/����	
		if (event_Think(index) == false) return;

		//���������˼����ִ�й������Ϊ
		switch (data.ai.think)
		{
		case AI_STATE_UPDATE:Think_StateUpdate(); break;//1��״̬����
 		case AI_MOVE_RAND:   Think_MoveRand();    break;//2������ƶ�
 		case AI_MOVE_ATK:    Think_MoveAtk();     break;//3�����򹥻�Ŀ��
 		case AI_MOVE_RETURN: Think_MoveReturn();  break;//4�����س�����
 		case AI_RELIVE_SET:  Think_SetReborn();   break;//5�����ʼ
 		case AI_RELIVE_DO:   Think_Reborn();      break;//6���������
// 		case AI_CLEAR:       Think_Clear();       break;//7������
		}
	}

	bool S_ROBOT::event_Think(const s32 index)
	{
		if (tmp.script == nullptr) return false;
		if (isLive() == false && (data.ai.think != AI_RELIVE_SET && data.ai.think != AI_RELIVE_DO))     return false;
		//ִ�й������Ϊ
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


		//1������Ŀ�� ������������
		if (data.ai.base.lock_type == N_FREE && tmp.script->isatk > 0)
		{
			event_SetLock(N_ROLE, index);
		}
		//2������������vs����Ŀ��
		if (data.ai.base.lock_type > N_FREE)
		{
			bool isUseSkill = skill_start();
			// this->data.ai.think = AI_MOVE_ATK;
			if(isUseSkill) return true;
		}
		//3��Ѳ��-����ƶ�
		if (tmp.script->range > 0)
		{
			if (tmp.temp_buffRun.isMove() && data.ai.isTimeMoveRND())
			{
				data.ai.think = AI_MOVE_RAND;
				return true;
			}
		}
		//4��״̬�ָ�
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


