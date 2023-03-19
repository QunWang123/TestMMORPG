#include "AppSkill.h"
#include "UserData.h"
#include "AppManager.h"
#include "MyCharacterBase.h"
#include "MyGameInstance.h"
#include "WorldScript.h"
#include "UDPClient.h"

namespace app
{
	AppSkill* __AppSkill = nullptr;

	AppSkill::AppSkill(){}

	AppSkill::~AppSkill(){}

	void AppSkill::onInit(){}
#pragma optimize( "", off )
	// 使用技能返回的错误信息，正常释放技能了不会走这里
	void onCMD_3000(net::TCPClient* tc)
	{
		int16 childcmd = 0;
		tc->read(childcmd);
		
		if (__MyCharacter != nullptr) __MyCharacter->onSkillCommand_Err(childcmd);

	}

	// 开始释放技能
	void onCMD_3100(net::TCPClient* tc)
	{
		int32 userindex = 0;
		int32 skillid = 0;
		int8 skilllevel = 0;
		int8 lock_type = 0;
		int32 lock_index = 0;
		FS_VECTOR tpos;
		FMemory::Memset(&tpos, 0, sizeof(FS_VECTOR));

		tc->read(userindex);
		tc->read(skillid);
		tc->read(skilllevel);
		tc->read(lock_type);
		tc->read(lock_index);
		tc->read(&tpos, sizeof(FS_VECTOR));

		FVector targetpos;
		targetpos.X = tpos.x;
		targetpos.Y = tpos.y;
		targetpos.Z = tpos.z;

		FString str = FString::Printf(TEXT("%d,%d,%d,%d,%d,%d,%d,%d"), 
			(int32)E_EVENT_TYPE::EVENT_SKILL, skillid, skilllevel, lock_type, lock_index,
			tpos.x, tpos.y, tpos.z);

		// 判断是不是自己发出的
		if (userindex == __myUserData.userindex)
		{
			__myUserData.events.event_List.Add(str);
			return;
		}
		FOTHER_ROLE_BASE* d = onLineDatas.Find(userindex);
		if (d == NULL) return;
		d->events.event_List.Add(str);
	}

	void onCMD_3200(net::TCPClient* tc)
	{
		int32 userindex = 0;
		int8 lock_type = 0;
		int32 lock_index = 0;
		int32 curhp = 0;
		int32 atk = 0;

		tc->read(userindex);
		tc->read(lock_type);
		tc->read(lock_index);
		tc->read(curhp);
		tc->read(atk);

		switch (lock_type)
		{
		case (int8)E_NODE_TYPE::N_ROLE:
		{
			if (lock_index == __myUserData.userindex)
			{
				__myUserData.base.life.hp = curhp;
				__MyCharacter->onSkillCommand_HP(curhp, atk);
				return;
			}
			FOTHER_ROLE_BASE* data = onLineDatas.Find(lock_index);
			if (data == NULL) return;
			data->hp = curhp;
			if (data->view == NULL) return;
			ACharacterBase* view = (ACharacterBase *)data->view;
			if (view == NULL)return;
			view->onSkillCommand_HP(curhp, atk);
		}
			break;
		case (int8)E_NODE_TYPE::N_MONSTER:
		case (int8)E_NODE_TYPE::N_PET:
		case (int8)E_NODE_TYPE::N_NPC:
		{
			FROBOT* robotdata = robot_Find(lock_index);
			if (robotdata == NULL) return;
			robotdata->data.hp = curhp;
			if (robotdata->view == NULL)return;
			ARobotBase* view = (ARobotBase*)robotdata->view;
			if (view == NULL) return;
			view->onSkillCommand_HP(curhp, atk);
		}
			break;
		default:
			break;
		}
	}

	void onCMD_4000(net::TCPClient* tc)
	{
		int32 userindex = 0;
		tc->read(userindex);

		FString str = FString::Printf(TEXT("%d"), (int32)E_EVENT_TYPE::EVENT_DEAD);

		if (userindex == __myUserData.userindex)
		{
			__myUserData.events.event_List.Add(str);
			return;
		}
		FOTHER_ROLE_BASE* data = onLineDatas.Find(userindex);
		if (data == NULL) return;
		data->events.event_List.Add(str);
	}

	void onCMD_4100(net::TCPClient* tc)
	{
		int16 childcmd = 0;
		tc->read(childcmd);

		FVector pos;
		if (__MyCharacter != NULL) __MyCharacter->onRebornCommand_Play(childcmd, pos);
	}

	void onCMD_4200(net::TCPClient* tc)
	{
		int32 userindex;
		int32 curhp;
		FVector targetpos;
		FS_VECTOR vpos;

		tc->read(userindex);
		tc->read(curhp);
		tc->read(&vpos, sizeof(FS_VECTOR));

		targetpos.X = vpos.x;
		targetpos.Y = vpos.y;
		targetpos.Z = vpos.z;

		if (userindex == __myUserData.userindex)
		{
			__myUserData.base.life.hp = curhp;
			if (__MyCharacter != NULL) __MyCharacter->onRebornCommand_Play(0, targetpos);
			return;
		}
		FOTHER_ROLE_BASE* other = onLineDatas.Find(userindex);
		if (other != NULL)
		{
			other->hp = curhp;
			other->pos = targetpos;
			if (other->view != NULL)
			{
				ACharacterBase* view = (ACharacterBase*)other->view;
				if (view == NULL) return;
				view->onRebornCommand_Play(0, targetpos);
			}
		}
	}

	void onCMD_5000(net::TCPClient* tc)
	{
		int32 userindex = 0;
		int32 buff_id = 0;
		int32 runningtime = 0;
		tc->read(userindex);
		tc->read(buff_id);
		tc->read(runningtime);

		if (buff_id >= MAX_BUFFSCRIPT_COUNT) return;
		
		if (userindex == __myUserData.userindex)
		{
			script::SCRIPT_BUFF_BASE* script_buff = script::fingScript_Buff(buff_id);
			if (script_buff != NULL)
			{
				__myUserData.tmp.buff_run.addState(script_buff->state);
				// int32 value = buff->state & E_BUFF_NOMOVE;
				// if (value == E_BUFF_NOMOVE) __myUserData.tmp.buff_run.addState(E_BUFF_NOMOVE);
				// value = buff->state & E_BUFF_NOSKILL;
				// if (value == E_BUFF_NOSKILL) __myUserData.tmp.buff_run.addState(E_BUFF_NOSKILL);
			}
			FUSER_ROLE_BUFF_BASE* buff = __myUserData.tmp.buff_run.findFree();
			// FUSER_ROLE_BUFF_BASE* buff = &__myUserData.tmp.buff_run.data[buff_id];
			if (buff == nullptr) return;
			buff->buff_id = buff_id;
			buff->runningtime = runningtime;
			__MyCharacter->onBuffCommand_Play(buff_id, runningtime);
			return;
		}
		FOTHER_ROLE_BASE* otheruser = onLineDatas.Find(userindex);
		if (otheruser == NULL || otheruser->view == NULL) return;

		FUSER_ROLE_BUFF_BASE* buff = __myUserData.tmp.buff_run.findFree();
		// FUSER_ROLE_BUFF_BASE* buff = &__myUserData.tmp.buff_run.data[buff_id];
		if (buff == nullptr) return;
		buff->buff_id = buff_id;
		buff->buff_id = runningtime;

		ACharacterBase* view = (ACharacterBase*)otheruser->view;
		if (view != NULL) view->onBuffCommand_Play(buff_id, runningtime);
	}

	bool AppSkill::onClientCommand(net::TCPClient* tc, const uint16 cmd)
	{
		switch (cmd)
		{
		case CMD_3000:
			onCMD_3000(tc);
			break;
		case CMD_3100:
			onCMD_3100(tc);
			break;
		case CMD_3200:
			onCMD_3200(tc);
			break;
		case CMD_4000:
			onCMD_4000(tc);
			break;
		case CMD_4100:
			onCMD_4100(tc);
			break;
		case CMD_4200:
			onCMD_4200(tc);
			break;
		case CMD_5000:
			onCMD_5000(tc);
			break;
		}
		return true;
	}

	void AppSkill::send_Skill(int32 skillindex, int lock_type, int lock_index, FVector targetpos)
	{
		FS_VECTOR tpos;
		tpos.x = targetpos.X;
		tpos.y = targetpos.Y;
		tpos.z = targetpos.Z;

#ifdef TEST_UDP
		S_SEND_3000  d;
		d.ID = func::__UDPClientInfo->ID;
//		d.cmd = CMD_65002;
		d.cmd = CMD_3000;
		d.skillindex = skillindex;
		d.lock_type = lock_type;
		d.lock_index = lock_index;
		d.targetpos = tpos;
		d.setSecure(func::__UDPClientInfo->RCode);
		// __udp->sendData(&d, sizeof(S_SEND_3000), SPT_UDP);
		__udp->sendData(&d, sizeof(S_SEND_3000), net::SPT_KCP);
		return;
#endif

		app::__TcpClient->begin(CMD_3000);
		app::__TcpClient->sss((int8)skillindex);
		app::__TcpClient->sss((int8)lock_type);
		app::__TcpClient->sss(lock_index);
		app::__TcpClient->sss(&tpos, sizeof(FS_VECTOR));
		app::__TcpClient->end();
	}

	void AppSkill::send_Reborn(int32 kind)
	{
		app::__TcpClient->begin(CMD_4100);
		app::__TcpClient->sss((int8)kind);
		app::__TcpClient->end();
	}

#ifdef TEST_UDP

	//3100 使用技能返回信息
	void onCMD_UDP_3100(UDPClient* udp)
	{
		S_RECV_3100  data;
		udp->readData(&data, sizeof(S_RECV_3100));


		FString str = FString::Printf(TEXT("%d,%d,%d,%d,%d,%d,%d,%d"), (int32)E_EVENT_TYPE::EVENT_SKILL,
			data.skillid, data.level, data.lock_type, data.lock_index, data.targetpos.x, data.targetpos.y, data.targetpos.z);

		//1 如果是自己
		if (data.userindex == __myUserData.userindex)
		{

			//FString ss = FString::Printf(TEXT("reve Skill  ..."));
			//if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 5, FColor::Emerald, ss);

			__myUserData.events.event_List.Add(str);
			return;
		}
		//2、如果是其他玩家
		FOTHER_ROLE_BASE* d = onLineDatas.Find(data.userindex);
		if (d == NULL) return;
		d->events.event_List.Add(str);

	}

	//udp
	void AppSkill::onClientCommand_Udp(UDPClient* udp, int32 cmd)
	{
		FString ss = FString::Printf(TEXT("reve Skill  ...%d"), cmd);
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 15, FColor::Emerald, ss);

		switch (cmd)
		{
		case CMD_3100:
			onCMD_UDP_3100(udp);
			break;
		}
	}
#endif

}



#pragma optimize( "", on )