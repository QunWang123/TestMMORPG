#include "WorldBC.h"
#include "WorldMap.h"
#include "AppGlobal.h"
#include "CommandData.h"

namespace app
{
	void TS_Broadcast::db_Disconnect(u32 userindex, u64 memid, u32 timelogin)
	{
		__TcpDB->begin(CMD_600);
		__TcpDB->sss(userindex);
		__TcpDB->sss(memid);
		__TcpDB->sss(timelogin);
		__TcpDB->end();
	}

	void TS_Broadcast::db_SaveStatusInfo(S_USER_BASE* user)
	{
		__TcpDB->begin(CMD_731);
		// __TcpDB->sss(user->node.index);
		__TcpDB->sss(user->tmp.userindex);
		__TcpDB->sss(user->mem.id);
		__TcpDB->sss(&user->role.base.status, sizeof(S_ROLE_BASE_STATUS));
		__TcpDB->end();
	}

	//901 切换地图 
	//902 环线
	//7200 单人副本
	//7300 多人副本
	void TS_Broadcast::db_ChangeMap(net::S_CLIENT_BASE* c, void* d, s32 userindex, u8 line, u32 mapid, u16 cmd, u16 childcmd)
	{
		if (d == nullptr) return;
		S_COMMAND_GATEBASE* data = (S_COMMAND_GATEBASE*)d;
		if (data == nullptr) return;

		S_LOAD_ROLE data2;
		data2.cmd = cmd;
		data2.user_connectid = data->user_connectid;
		data2.userindex = userindex;
		data2.memid = data->memid;
		data2.mapid = mapid;
		data2.server_connectid = c->ID;
		data2.server_clientid = c->clientID;
		data2.line = line;


		__TcpDB->begin(cmd);
		__TcpDB->sss(childcmd);
		__TcpDB->sss(&data2, sizeof(S_LOAD_ROLE));
		__TcpDB->end();
	}

	//CMD_800 背包更新道具（可加可减）
	void TS_Broadcast::db_SendUpdateProp(S_USER_BASE* user, u8 bagpos, bool iscount)
	{
		if (bagpos >= USER_MAX_BAG) return;
		S_ROLE_PROP* prop = &user->role.stand.bag.bags[bagpos];

		__TcpDB->begin(CMD_800);
		__TcpDB->sss(user->node.index);
		__TcpDB->sss(user->mem.id);
		__TcpDB->sss(bagpos); //背包位置
		__TcpDB->sss(iscount);//更新数量还是全部数据？
		//只更新数量
		if (iscount)
		{
			__TcpDB->sss(prop->base.id);
			__TcpDB->sss(prop->base.count); //0 代表删除
		}
		else
		{
			__TcpDB->sss(prop, prop->sendSize());
		}

		__TcpDB->end();
	}

	void TS_Broadcast::db_SaveLevelUP(S_USER_BASE* user)
	{
		__TcpDB->begin(CMD_750);
		__TcpDB->sss(user->node.index);
		__TcpDB->sss(user->mem.id);
		__TcpDB->sss(&user->role.base.exp, sizeof(S_ROLE_BASE_EXP));
		__TcpDB->end();
	}
	void TS_Broadcast::db_SaveCompat(S_USER_BASE* user, u8 bagpos, u8 equippos, u8 kind)
	{
		__TcpDB->begin(CMD_810);
		__TcpDB->sss(user->node.index);
		__TcpDB->sss(user->mem.id);
		__TcpDB->sss(kind);		//0 穿 1脱  2装备换位置
		__TcpDB->sss(bagpos);	//背包位置
		__TcpDB->sss(equippos);	//战斗装备中位置
		__TcpDB->end();
	}

	void TS_Broadcast::db_SaveSwap(S_USER_BASE* user, u8 equippos1, u8 equippos2)
	{
		__TcpDB->begin(CMD_890);
		__TcpDB->sss(user->node.index);
		__TcpDB->sss(user->mem.id);
		__TcpDB->sss(equippos1);
		__TcpDB->sss(equippos2);
		__TcpDB->end();
	}

	void TS_Broadcast::db_SaveGold(S_USER_BASE* user)
	{
		__TcpDB->begin(CMD_760);
		__TcpDB->sss(user->tmp.userindex);
		__TcpDB->sss(user->mem.id);
		__TcpDB->sss(&user->role.base.econ, sizeof(S_ROLE_BASE_ECON));
		__TcpDB->end();
	}
	void TS_Broadcast::db_SendExp(S_USER_BASE* user)
	{
		__TcpDB->begin(CMD_740);
		__TcpDB->sss(user->tmp.userindex);
		__TcpDB->sss(user->mem.id);
		__TcpDB->sss(user->role.base.exp.currexp);
		__TcpDB->end();
	}
}