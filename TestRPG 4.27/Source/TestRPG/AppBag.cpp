#include "AppBag.h"
#include "CommandData.h"
#include "UserData.h"
#include "BlueprintFunction.h"
#include "MyCharacterBase.h"
#include "AppManager.h"
#include "MyGameInstance.h"

namespace app
{
	// UObject
	AppBag* __AppBag = nullptr;
	TMap<int32, int32> __SendTimes_Combat;				// 控制申请玩家数据时间

	bool isSendRoleCombat(int32 userindex)
	{
		auto it = __SendTimes_Combat.Find(userindex);
		if (it != nullptr)
		{
			int32 ftime = *it;
			ftime = __AppGameInstance->GetTimeSeconds() - ftime;
			if (ftime > 0 && ftime < 2000)
			{
				FString ss = FString::Printf(TEXT("GetOtherData False...%d"), ftime);
				if (GEngine)
					GEngine->AddOnScreenDebugMessage((uint64)-1, 5, FColor::Emerald, ss);
				return false;
			}
			__SendTimes_Combat.Remove(userindex);
			return true;
		}
		__SendTimes_Combat.Add(userindex, __AppGameInstance->GetTimeSeconds());
		return true;
	}

	AppBag::AppBag()
	{

	}

	AppBag::~AppBag()
	{

	}

	void AppBag::onInit()
	{

	}

	void onCMD_800(net::TCPClient* tc)
	{
		uint8 bagpos = 0;
		bool iscount = false;
		tc->read(bagpos);
		tc->read(iscount);

		if (bagpos >= __myUserData.stand.bag.num) return;
		if (iscount)
		{
			uint16 count = 0;
			tc->read(count);
			// 数据在这里处理就行，UI只管显示
			if (count == 0)
			{
				__myUserData.stand.bag.bags[bagpos].reset();
			}
			else
			{
				__myUserData.stand.bag.bags[bagpos].base.count = count;
			}
			// 通知更新单个，只需更新数量
			if (__AppGameInstance) __AppGameInstance->onBagCommand(CMD_800, __myUserData.userindex, bagpos, -1, 0);
			// __MyCharacter->updateBagSpecifyGridNum(bagpos);
		}
		else
		{
			__myUserData.stand.bag.bags[bagpos] = UBlueprintFunction::read_FROLE_PROP();
			// 需要整个格子都更新（数量、图片等）
			// __MyCharacter->updateBagSpecifyGrid(bagpos);
			if (__AppGameInstance) __AppGameInstance->onBagCommand(CMD_800, __myUserData.userindex, bagpos, -1, 1);
		}
	}

	void onCMD_810(net::TCPClient* tc)
	{
		uint8 kind = 0;
		uint8 bagpos = 0;
		uint8 equippos = 0;

		tc->read(kind);
		tc->read(bagpos);
		tc->read(equippos);

		FROLE_PROP* bag_prop = nullptr;
		if (kind == 2)
		{
			bag_prop = &__myUserData.stand.combat.equips[bagpos];
		}
		else
		{
			bag_prop = &__myUserData.stand.bag.bags[bagpos];
		}
			
		FROLE_PROP* equip_prop = &__myUserData.stand.combat.equips[equippos];

		switch (kind)
		{
		case 0://穿装备
		case 2://装备换位置
		{
			FROLE_PROP temp;
			temp.reset();
			FMemory::Memcpy(&temp, equip_prop, sizeof(FROLE_PROP));
			FMemory::Memcpy(equip_prop, bag_prop, sizeof(FROLE_PROP));
			FMemory::Memcpy(bag_prop, &temp, sizeof(FROLE_PROP));
		}
		break;
		case 1://脱装备 
			FMemory::Memcpy(bag_prop, equip_prop, sizeof(FROLE_PROP));
			equip_prop->reset();
			break;
		}
		if (__MyCharacter)
		{
			// kind = 0是更新装备栏，1是刷新攻击数据
			if (__AppGameInstance) __AppGameInstance->onBagCommand(CMD_810, __myUserData.userindex, bagpos, equippos, kind);
// 			__MyCharacter->updateBagSpecifyGrid(bagpos);
// 			__MyCharacter->updateCombatSpecifyGrid(equippos);
		}
	}

	void onCMD_820(net::TCPClient* tc)
	{
		uint16 childcmd = 0;
		int32 userindex = 0;
		uint8 num = 0;

		tc->read(childcmd);
		tc->read(userindex);
		tc->read(num);
		if (childcmd != 0) return;

		FROLE_STAND_COMBAT* combat = nullptr;
		// 判断是不是自己发出的
		if (userindex == __myUserData.userindex)
		{
			combat = &__myUserData.stand.combat;
		}
		else
		{
			FOTHER_ROLE_BASE* data = onLineDatas.Find(userindex);
			if (data)
			{
				combat = &data->combat;
			}
		}
		if (combat && num > 0)
		{
			for (uint8 i = 0; i < num; i++)
			{
				uint8 bagpos = 0;
				tc->read(bagpos);
				if (bagpos < MAX_USER_EQUIP)
				{
					combat->equips[bagpos] = UBlueprintFunction::read_FROLE_PROP();
				}
			}
		}
		if (userindex == __myUserData.userindex)
		{
			if (__AppGameInstance) __AppGameInstance->onBagCommand(CMD_820, __myUserData.userindex, -1, -1, 0);
			// __MyCharacter->updateAllCombatGrid();
		}
		else
		{
			if (__AppGameInstance) __AppGameInstance->onBagCommand(CMD_820, userindex, -1, -1, 1);
			// 其他玩家调用的，在appgameinstance中实现一下吧
		}
	}

	// 获取背包全部数据
	void onCMD_830(net::TCPClient* tc)
	{
		uint16 childcmd = 0;
		uint8 num = 0;
		tc->read(childcmd);

		if (childcmd != 0) return;

		tc->read(__myUserData.stand.bag.num);
		tc->read(num);
		if (num > 0)
		{
			for (uint8 i = 0; i < num; i++)
			{
				uint8 bagpos = 0;
				tc->read(bagpos);
				if (bagpos < __myUserData.stand.bag.num)
				{
					__myUserData.stand.bag.bags[bagpos] = UBlueprintFunction::read_FROLE_PROP();
				}
			}
		}
		if (__AppGameInstance) __AppGameInstance->onBagCommand(CMD_830, __myUserData.userindex, -1, -1, -1);
		// __MyCharacter->updateAllBagGrid();
	}

	bool AppBag::onClientCommand(net::TCPClient* tc, const uint16 cmd)
	{
		switch (cmd)
		{
		case CMD_800:
			onCMD_800(tc);
			break;
		case CMD_810:
			onCMD_810(tc);
			break;
		case CMD_820:
			onCMD_820(tc);
			break;
		case CMD_830:
			onCMD_830(tc);
			break;
		case CMD_840:
		case CMD_850:
		case CMD_860:
		case CMD_870:
			break;
		}
		return true;
	}

	void AppBag::send_GetCombat(int32 userindex)
	{
		// if (!isSendRoleCombat(userindex)) return;
		__TcpClient->begin(CMD_820);
		__TcpClient->sss(userindex);
		__TcpClient->end();
	}

	void AppBag::send_GetBag()
	{
		__TcpClient->begin(CMD_830);
		__TcpClient->end();
	}

	void AppBag::send_CombaineBag()
	{
		__TcpClient->begin(CMD_840);
		__TcpClient->end();
	}

	void AppBag::send_EquipUp(int32 pos, int32 propid)
	{
		__TcpClient->begin(CMD_850);
		__TcpClient->sss((int8)pos);
		__TcpClient->sss(propid);
		__TcpClient->end();
	}

	void AppBag::send_EquipDown(int32 pos, int32 propid)
	{
		__TcpClient->begin(CMD_860);
		__TcpClient->sss((int8)pos);
		__TcpClient->sss(propid);
		__TcpClient->end();
	}

	void AppBag::send_ItemSell(int32 pos, int32 propid)
	{
		__TcpClient->begin(CMD_870);
		__TcpClient->sss((int8)pos);
		__TcpClient->sss(propid);
		__TcpClient->end();
	}

	void AppBag::send_Swap(int32 pos1, int32 propid1, int32 pos2, int32 propid2)
	{
		__TcpClient->begin(CMD_890);
		__TcpClient->sss((int8)pos1);
		__TcpClient->sss(propid1);
		__TcpClient->sss((int8)pos2);
		__TcpClient->sss(propid2);
		__TcpClient->end();
	}
}


