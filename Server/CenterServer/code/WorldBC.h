
#ifndef ____WORLDBC_H
#define ____WORLDBC_H

#include  "APPGlobal.h"
#include "WorldData.h"

namespace app
{
	//team 
	//gate
	extern void nf_GateServer_TeamSelf(S_USER_TEAM* user, const u32 cid, bool isleader);
	extern void bc_GateServer_TeamLeave(const u32 teamindex, u8 mateindex);
	extern void bc_GateServer_TeamLeader(const u32 roomid, u8 mateindex);
	extern void  bc_GateServer_TeamJoin(net::S_CLIENT_BASE* c, S_USER_TEAM* user, S_TEAM_BASE* team);


	//game
	extern void bc_GameServer_TeamData();
	extern void bc_GameServer_TeamData(const u32 roomid);
	extern void nf_GameServer_TeamLeave(S_USER_TEAM* user);
	extern void nf_GameServer_TeamSelf(S_USER_TEAM* user);

	//copy
	//extern void bc_GameServer_CopyChangeMap(u32 teamid, u32 mapid);
	//extern void nf_GameServer_CopyChangeMapOne(u32 centerindex, s64 memid, u32 mapid);
	//extern void nf_GameServer_CopyStart(void* room);
	//extern void nf_GameServer_CopyOver(u32 copyid, u32 mapid);
	//extern void nf_GameServer_CopyEntry(u32 mapid,u32 gameindex, s64 mid, s32 copyid);
}

#endif