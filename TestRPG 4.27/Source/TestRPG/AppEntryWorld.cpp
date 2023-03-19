#include "AppEntryWorld.h"
#include "CommandData.h"
#include "AppManager.h"
#include "MyGameInstance.h"
#include "Engine/Engine.h"
#include "BlueprintFunction.h"
#include "MyCharacterBase.h"

namespace app
{
	AppEntryWorld* __AppEntryWorld = nullptr;
	TMap<int32, int32> __SendTimes_User;				// 控制申请玩家数据时间
	int32 temp_ChangTime = 0;

	bool isSendRoleData(int32 userindex)
	{
		auto it = __SendTimes_User.Find(userindex);
		if (it != nullptr)
		{
			int32 ftime = *it;
			ftime = __AppGameInstance->GetTimeSeconds() - ftime;
			if (ftime > 0 && ftime < 5000)
			{
				FString ss = FString::Printf(TEXT("GetOtherData False...%d"), ftime);
				if(GEngine) 
					GEngine->AddOnScreenDebugMessage((uint64)-1, 5, FColor::Emerald, ss);
				return false;
			}
			// *it = __AppGameInstance->GetTimeSeconds();
			__SendTimes_User.Remove(userindex);
			return true;
		}
		__SendTimes_User.Add(userindex, __AppGameInstance->GetTimeSeconds());
		return true;
	}

	AppEntryWorld::AppEntryWorld()
	{

	}

	AppEntryWorld::~AppEntryWorld()
	{

	}

	void AppEntryWorld::onInit()
	{

	}

	void onCMD_600(net::TCPClient* tc)
	{
		int32 userindex = 0;
		tc->read(userindex);

		FOTHER_ROLE_BASE* data = onLineDatas.Find(userindex);
		if (data != NULL)
		{
			onLineDatas.Remove(userindex);
		}

		if (__AppGameInstance != NULL) __AppGameInstance->onEntryWorldCommand(CMD_600, userindex, 0);
	}

	void onCMD_700(net::TCPClient* tc)
	{
		uint16 childcmd = 0;
		FOTHER_ROLE_BASE role;

		tc->read(childcmd);
		if (childcmd != 0)
		{
			if (__AppGameInstance != nullptr)
			{
				__AppGameInstance->onOtherRoleDataCommand(childcmd, role);
			}
			return;
		}
		uint16 level = 0;
		int16 face = 0;
		FS_VECTOR pos;

		tc->read(role.userindex);
		tc->read(role.roleid);
		tc->read(role.job);
		tc->read(role.sex);
		role.nick = UBlueprintFunction::read_FString_len(USER_MAX_NICK);

		tc->read(face);
		tc->read(&pos, 12);
		tc->read(level);
		tc->read(role.hp);

		if (level < 1) level = 1;
		role.pos.X = pos.x;
		role.pos.Y = pos.y;
		role.pos.Z = pos.z;
		role.level = level;
		role.face = face;
		role.reset();

		FOTHER_ROLE_BASE* data = onLineDatas.Find(role.userindex);
		if (data != NULL)
		{
			// 蓝图中移除显示对象
			if (__AppGameInstance != NULL) __AppGameInstance->onEntryWorldCommand(CMD_600, data->userindex, 0);
			onLineDatas.Remove(role.userindex);
		}
		onLineDatas.Add(role.userindex, role);
		if (__AppGameInstance != NULL) __AppGameInstance->onOtherRoleDataCommand(childcmd, role);
	
// 		if (role.hp <= 0)
// 		{
// 			FString str = FString::Printf(TEXT("%d"), (int32)E_EVENT_TYPE::EVENT_DEAD);
// 			role.events.event_List.Add(str);
// 		}
	}

	void onCMD_710(net::TCPClient* tc)
	{
		int32 userindex = 0;
		int32 hp = 0;
		tc->read(userindex);
		tc->read(hp);
		if (userindex == __myUserData.userindex)
		{
			__myUserData.base.life.hp = hp;
			return;
		}
		if (__AppGameInstance != NULL) __AppGameInstance->onEntryWorldCommand(CMD_710, userindex, hp);
	}

	void onCMD_720(net::TCPClient* tc)
	{
		int32 userindex = 0;
		int32 mp = 0;
		tc->read(userindex);
		tc->read(mp);
		if (userindex == __myUserData.userindex)
		{
			__myUserData.base.life.mp = mp;
			return;
		}
		if (__AppGameInstance != NULL) __AppGameInstance->onEntryWorldCommand(CMD_720, userindex, mp);
	}

	void onCMD_730(net::TCPClient* tc)
	{
		int32 userindex = 0;
		int8 state = 0;
		tc->read(userindex);
		tc->read(state);

		if (__AppGameInstance != NULL) __AppGameInstance->onEntryWorldCommand(CMD_730, userindex, state);
	}

	//740 更新经验
	void onCMD_740(net::TCPClient* tc)
	{
		tc->read(__myUserData.base.exp.currexp);
		__MyCharacter->onCommand_CurrExp(__myUserData.base.exp.currexp);
		FString ss = FString::Printf(TEXT("my curexp:%d  ...%d"), __myUserData.base.exp.currexp);
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 35, FColor::Emerald, ss);
	}
	//750 升级
	void onCMD_750(net::TCPClient* tc)
	{
		int32 userindex = 0;
		int16 level;
		tc->read(userindex);
		tc->read(level);

		if (userindex == __myUserData.userindex)
		{
			__myUserData.base.exp.level = level;
			__MyCharacter->onCommand_Level(level);
			return;
		}
		
		FOTHER_ROLE_BASE* data = onLineDatas.Find(userindex);
		if (data == NULL) return;
		if (data->view == NULL) return;
		data->level = level;
		ACharacterBase* view = (ACharacterBase*)data->view;
		view->onCommand_Level(level);
	}
	//760 更新自己的经济
	void onCMD_760(net::TCPClient* tc)
	{
		tc->read(__myUserData.base.econ.gold);
		tc->read(__myUserData.base.econ.diamonds);

		__MyCharacter->onCommand_Economy(__myUserData.base.econ.gold, __myUserData.base.econ.diamonds);
		FString ss = FString::Printf(TEXT("my econ:%d %d"), __myUserData.base.econ.gold, __myUserData.base.econ.diamonds);
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 35, FColor::Emerald, ss);
	}

	void onCMD_770(net::TCPClient* tc)
	{
		int32 userindex;
		tc->read(userindex);

		if (userindex == __myUserData.userindex)
		{
			tc->read(&__myUserData.atk, sizeof(FSPRITE_ATTACK));
			if (__AppGameInstance) __AppGameInstance->onBagCommand(CMD_880, userindex, -1, -1, 0);
			return;
		}
		FOTHER_ROLE_BASE* data = onLineDatas.Find(userindex);
		if (data == NULL) return;
		tc->read(&data->atk, sizeof(FSPRITE_ATTACK));
		if (__AppGameInstance) __AppGameInstance->onBagCommand(CMD_880, userindex, -1, -1, 1);
	}

	// 900正常情况下是不会收到的
	void onCMD_900(net::TCPClient* tc)
	{
		uint16 childcmd = 0;
		tc->read(childcmd);
		if (__AppGameInstance != NULL) __AppGameInstance->onEntryWorldCommand(CMD_900, childcmd, 0);
	}
	void onCMD_901(net::TCPClient* tc)
	{
		uint16 childcmd = 0;
		tc->read(childcmd);
		if (childcmd != 0)
		{
			FString ss = FString::Printf(TEXT("changemap err:%d"), childcmd);
			if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 115, FColor::Emerald, ss);
			return;
		}
		tc->read(__myUserData.base.status.mapid);
		tc->read(&__myUserData.base.status.pos, 12);

		InitMapInfo(__myUserData.base.status.mapid);
		if (__AppGameInstance != NULL) __AppGameInstance->onChangeMapCommand(CMD_901, childcmd, __myUserData.base.status.mapid);
	}
	void onCMD_902(net::TCPClient* tc)
	{
		uint16 childcmd = 0;
		tc->read(childcmd);
		
		if (__AppGameInstance != NULL) __AppGameInstance->onChangeMapCommand(CMD_902, childcmd, __myUserData.base.status.mapid);
	}
	// 收到服务器返回指令集
	bool AppEntryWorld::onClientCommand(net::TCPClient* tc, const uint16 cmd)
	{
		switch (cmd)
		{
		case CMD_600:
			onCMD_600(tc);
			break;
		case CMD_700:
			onCMD_700(tc);
			break;
		case CMD_710:
			onCMD_710(tc);
			break;
		case CMD_720:
			onCMD_720(tc);
			break;
		case CMD_730:
			onCMD_730(tc);
			break;
		case CMD_740: 
			onCMD_740(tc); 
			break;
		case CMD_750:
			onCMD_750(tc); 
			break;
		case CMD_760: 
			onCMD_760(tc);  
			break;
		case CMD_770:
			onCMD_770(tc);
			break;
		case CMD_900:
			onCMD_900(tc);
			break;
		case CMD_901:
			onCMD_901(tc);
			break;
		case CMD_902:
			onCMD_902(tc);
			break;
		}
		return true;
	}

	// 进入地图后，游戏BP_MyCharacter生成时调用的
	void AppEntryWorld::send_EntryWorld()
	{
		__TcpClient->begin(CMD_900);
		__TcpClient->end();
	}

	// 获得710.730这样的定时广播时，由于没有targetindex，因此需要获取
	void AppEntryWorld::send_GetOtherRoleData(int32 userindex)
	{
		if (!isSendRoleData(userindex)) return;
		__TcpClient->begin(CMD_700);
		__TcpClient->sss(userindex);
		__TcpClient->end();
	}

	void AppEntryWorld::send_ChangeMap(int32 mapid)
	{
		if (!__AppGameInstance) return;
		if (temp_ChangTime > __AppGameInstance->GetTimeSeconds() || temp_ChangTime == 0)
		{
			// 纠错用的，窗口重开的时候__AppGameInstance->GetTimeSeconds()会被设为0，但temp_ChangTime却没有
			temp_ChangTime = __AppGameInstance->GetTimeSeconds();
		}
		if (__AppGameInstance->GetTimeSeconds() - temp_ChangTime < 3000) return;
		temp_ChangTime = __AppGameInstance->GetTimeSeconds();
		clearAllRobot();
		clearAllUser();
		__TcpClient->begin(CMD_901);
		__TcpClient->sss(mapid);
		__TcpClient->end();
	}

	void AppEntryWorld::send_ChangeLine(uint8 line)
	{
		if (!__AppGameInstance) return;
		if (temp_ChangTime > __AppGameInstance->GetTimeSeconds() || temp_ChangTime == 0)
		{
			// 纠错用的，窗口重开的时候__AppGameInstance->GetTimeSeconds()会被设为0，但temp_ChangTime却没有
			temp_ChangTime = __AppGameInstance->GetTimeSeconds();
		}
		if (__AppGameInstance->GetTimeSeconds() - temp_ChangTime < 3000) return;
		temp_ChangTime = __AppGameInstance->GetTimeSeconds();
		__TcpClient->begin(CMD_902);
		clearAllRobot();
		clearAllUser();
		__TcpClient->sss(line);
		__TcpClient->end();
	}
}