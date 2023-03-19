#include "AppRobot.h"
#include "MyGameInstance.h"
#include "AppManager.h"
#include "RobotData.h"
#include "RobotBase.h"
#include "WorldTools.h"
#include "WorldScript.h"
#include "UserData.h"
#include "MyCharacterBase.h"

namespace app
{
	AppRobot* __AppRobot = nullptr;
	TMap<int32, int32> __SendTimes_Robot;//控制申请怪物数据时间

	//是否能发送请求数据 主要用于控制发送时间间隔不能太短
	bool isSendRobotData(int32 robotindex)
	{
		auto it = __SendTimes_Robot.Find(robotindex);
		if (it != nullptr)
		{
			int32 ftime = *it;
			ftime = __AppGameInstance->GetTimeSeconds() - ftime;
			// 发送间隔不能小于1秒
			if (ftime < 1000 && ftime > 0)
			{
				//FString ss = FString::Printf(TEXT("getRobotdata is faster...%d"), ftime);
				//if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 5, FColor::Emerald, ss);
				return false;
			}

			__SendTimes_Robot.Remove(robotindex);
		}

		__SendTimes_Robot.Add(robotindex, __AppGameInstance->GetTimeSeconds());
		return true;
	}

	AppRobot::AppRobot(){}

	AppRobot::~AppRobot(){}

	void onCMD_8000(net::TCPClient* tcp)
	{
		FROBOT robotdata;
		robotdata.reset();
		uint16 childcmd = 0;

		tcp->read(childcmd);
		if (childcmd != 0)
		{
			FString ss = FString::Printf(TEXT("getRobotData err... %d\n"), childcmd);
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Emerald, ss);
			return;
		}
		tcp->read(&robotdata, 21);

		// 设置位置
		gridToPos(&robotdata.data.grid, &robotdata.data.pos, &__CurMapInfo.leftpos);

		FROBOT* data = robot_Find(robotdata.data.robotindex);
		if (data != nullptr)
		{
			destroyRobot(data);
// 			ARobotBase* view = (ARobotBase*)data->view;
// 			if (view != nullptr) view->Destroy();
// 			robotView_Remove(robotdata.data.robotindex);
// 			robot_Remove(robotdata.data.robotindex);
		}
		// 设置最大生命和昵称
		auto d = script::findScript_Monster(robotdata.data.id);
		if (d)
		{
			robotdata.data.maxhp = d->maxhp;
			robotdata.data.nick = d->nick;
		}

		robotdata.temp.temp_HeartTime = __AppGameInstance->GetTimeSeconds();
		robot_Add(robotdata.data.robotindex, &robotdata);
		if (__AppGameInstance != nullptr) __AppGameInstance->onRobotDataCommand(0, robotdata.data);
	}

	void onCMD_8100(net::TCPClient* tcp, const uint16 cmd)
	{
		int32 robotindex = 0;
		tcp->read(robotindex);

		FROBOT* robotdata = robot_Find(robotindex);
		if (robotdata == NULL)
		{
// 			FString ss = FString::Printf(TEXT("onCMD_%d err... %d,try to get!"), cmd, robotindex);
// 			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Emerald, ss);
			AppRobot::send_RobotData(robotindex);
			return;
		}
		
		robotdata->temp.temp_HeartTime = __AppGameInstance->GetTimeSeconds();

		ARobotBase* view = (ARobotBase*)robotdata->view;
		if (view != NULL && view->rindex != robotdata->data.robotindex)
		{
			FString ss = FString::Printf(TEXT("robot state ...%d %d\n"), robotdata->data.robotindex, view->rindex);
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Emerald, ss);
			view->rindex = robotdata->data.robotindex;
		}

		switch (cmd)
		{
		case CMD_8100:
			break;
		case CMD_8200:
		{
			tcp->read(robotdata->data.hp);
			if (view) view->onSkillCommand_HP(robotdata->data.hp, -99);
		}
			break;
		case CMD_8300:
		{
			uint8 state = 0;
			tcp->read(state);

			// 4.死亡
			if (state == 4)
			{
				FString ss = FString::Printf(TEXT("robot dead...%d %d\n"), cmd, robotindex);
				if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Emerald, ss);
				FString str = FString::Printf(TEXT("%d"), (int32)E_EVENT_TYPE::EVENT_DEAD);
				robotdata->events.event_List.Add(str);
			}
		}
			break;
		case CMD_8400:
		{
			FGRID_BASE grid;
			tcp->read(&grid, sizeof(FGRID_BASE));
			FString str = FString::Printf(TEXT("%d,%d,%d"), (int32)E_EVENT_TYPE::EVENT_MOVE, grid.row, grid.col);
			robotdata->events.event_List.Add(str);
		}
			break;
		case CMD_8500:
		{
			int32 skill_index = 0;
			int8 skill_level = 0;
			int8 lock_type = 0;
			int32 lock_index = 0;
			tcp->read(skill_index);
			tcp->read(skill_level);
			tcp->read(lock_type);
			tcp->read(lock_index);

			FString str = FString::Printf(TEXT("%d,%d,%d,%d,%d"),
				(int32)E_EVENT_TYPE::EVENT_SKILL, skill_index, skill_level, lock_type, lock_index);
			robotdata->events.event_List.Add(str);
		}
			break;
		}
	}

	void onCMD_8700(net::TCPClient* tcp)
	{
		int32 skill_robotindex = 0;
		int8 lock_type = 0;
		int32 skill_lockindex = 0;
		int32 curhp = 0;
		int32 atk = 0;
		tcp->read(skill_robotindex);
		tcp->read(lock_type);
		tcp->read(skill_lockindex);
		tcp->read(curhp);
		tcp->read(atk);

		switch (lock_type)
		{
		case (int8)E_NODE_TYPE::N_ROLE:
		{
			if (skill_lockindex == __myUserData.userindex)
			{
				__myUserData.base.life.hp = curhp;
				__MyCharacter->onSkillCommand_HP(curhp, atk);
				return;
			}
			FOTHER_ROLE_BASE* data = onLineDatas.Find(skill_lockindex);
			if (data == NULL) return;
			data->hp = curhp;
			if (data->view == NULL) return;
			ACharacterBase* view = (ACharacterBase *)data->view;
			if (view == NULL)return;
			view->onSkillCommand_HP(curhp, atk);
		}
			break;
		case (int8)E_NODE_TYPE::N_PET:
		case (int8)E_NODE_TYPE::N_MONSTER:
		case (int8)E_NODE_TYPE::N_NPC:
		{
			FROBOT* robotdata = robot_Find(skill_lockindex);
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

	void onCMD_8800(net::TCPClient* tcp)
	{
		int32 buff_robotindex = 0;
		int32 buff_id = 0;
		int32 buff_runningtime = 0;

		tcp->read(buff_robotindex);
		tcp->read(buff_id);
		tcp->read(buff_runningtime);

		if (buff_id >= MAX_BUFFSCRIPT_COUNT) return;

		FROBOT* robotdata = robot_Find(buff_robotindex);
		if (robotdata == NULL || robotdata->view == NULL) return;

		script::SCRIPT_BUFF_BASE* script_buff = script::fingScript_Buff(buff_id);
		if (script_buff != NULL)
		{
			robotdata->buff_run.addState(script_buff->state);
		}

		FUSER_ROLE_BUFF_BASE* buff = robotdata->buff_run.findFree();
		if (buff == NULL) return;
		buff->buff_id = buff_id;
		buff->runningtime = buff_runningtime;

		ARobotBase* robot = (ARobotBase*)robotdata->view;
		if (robot)robot->onBuffCommand_Play(buff_id, buff_runningtime);

	}

	bool AppRobot::onClientCommand(net::TCPClient* tc, const uint16 cmd)
	{
		switch (cmd)
		{
		case CMD_8000:
			onCMD_8000(tc);
			break;
		case CMD_8100:
		case CMD_8200:
		case CMD_8300:
		case CMD_8400:
		case CMD_8500:
			onCMD_8100(tc, cmd);
			break;
		case CMD_8700:
			onCMD_8700(tc);
			break;
		case CMD_8800:
			onCMD_8800(tc);
			break;
		}
		return true;
	}

	

	void AppRobot::send_RobotData(int32 robotindex)
	{
		if (isSendRobotData(robotindex) == false) return;

		app::__TcpClient->begin(CMD_8000);
		app::__TcpClient->sss(robotindex);
		app::__TcpClient->end();
	}
}



