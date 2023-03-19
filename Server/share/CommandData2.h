#ifndef ____COMMANDDATA_CHAT_H
#define ____COMMANDDATA_CHAT_H

#include "CommandData.h"


namespace app
{

#pragma pack(push, packing)
#pragma pack(1)

	//900 玩家进入
	struct S_CMD_USER_ENTRY :S_COMMAND_GATEBASE
	{
		this_constructor(S_CMD_USER_ENTRY)
		u8   line;//所在线路
		u32  userindex;
		s32  mapid;//当前所在地图ID
		u16  level;//等级
		char name[USER_MAX_MEMBER];   //账号 
		char nick[USER_MAX_MEMBER];
	};

	struct S_CMD_COPY :S_COMMAND_GATEBASE
	{
		this_constructor(S_CMD_COPY)
		u32  userindex;
		s32  mapid;
		s32  roomindex;
		s32  teamindex;
	};

#pragma pack(pop, packing)
}
#endif