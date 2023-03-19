#include "AppTeam.h"

#include  "CommandData2.h"
#include  "ShareFunction.h"
#include  "UserManager.h"
#include  "WorldBC.h"
// #include <LogFile.h>

namespace app
{
	//7300 开始副本
	void onStartCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_CMD_COPY data;
		ts->read(c->ID, &data, sizeof(S_CMD_COPY));

		auto user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onStartCopy 没有找到玩家数据...%d %lld\n", data.userindex, data.memid);
			return;
		}
		if(user->tmp.temp_TeamIndex != data.teamindex)
		{
			LOG_MESSAGE("onStartCopy 队伍数据错误...%d %d\n", user->tmp.temp_TeamIndex, data.teamindex);
			return;
		}

		LOG_MESSAGE("[GameServer]  onStartCopy...index:%d/%lld mapid:%d\n", data.userindex, data.memid, data.mapid);


		auto team = __AppTeam->findTeam(data.teamindex);
		if (team == nullptr) return;

		bc_GameServer_TeamData(team->index);

		//广播给各个队员 副本开启
		for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
		{
			auto mate = team->findMate(i);
			if (mate == nullptr) continue;
			if (mate->memid == data.memid && mate->userindex == data.userindex) continue;
			auto other = __UserManager->findUser(mate->userindex, mate->memid);
			if (other == nullptr) continue;


			S_CMD_COPY data2;
			data2.userindex = mate->userindex;
			data2.memid = mate->memid;
			data2.roomindex = data.roomindex;
			data2.mapid = data.mapid;
			data2.teamindex = team->index;
			data2.user_connectid = other->tmp.user_connectid;

			//auto c1 = __TcpServer->client(other->tmp.server_connectid, other->tmp.server_clientid);
			//if (c1 == nullptr) continue;

			//__TcpServer->begin(c1->ID, CMD_7301);
			//__TcpServer->sss(c1->ID, &data2, sizeof(S_CMD_COPY));
			//__TcpServer->end(c1->ID);

			//这里通知玩家所在的服务器 开始进入副本
			net::S_CLIENT_BASE* c2 = findGameServer_Connection(other->tmp.line, other->status.mapid);
			if (c2 == nullptr) continue;

			__TcpServer->begin(c2->ID, CMD_7301);
			__TcpServer->sss(c2->ID, &data2, sizeof(S_CMD_COPY));
			__TcpServer->end(c2->ID);
		}
	}

	//7400 离开副本-离开队伍
	void onLeaveCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_CMD_COPY data;
		ts->read(c->ID, &data, sizeof(S_CMD_COPY));

		auto user = __UserManager->findUser(data.userindex, data.memid);
		if(user == nullptr)
		{
			LOG_MESSAGE("onLeaveCopy  error...%d %lld line:%d \n", data.userindex, data.memid, __LINE__);
			return;
		}
		if(user->tmp.temp_TeamIndex != data.teamindex) 
		{
			LOG_MESSAGE("onLeaveCopy  error...%d %d line:%d \n", user->tmp.temp_TeamIndex,data.teamindex, __LINE__);
			return;
		}
		LOG_MESSAGE("[GameServer]  onLeaveCopy...index:%d/%lld mapid:%d\n", data.userindex, data.memid, data.mapid);

		__AppTeam->setTeamLeave(user, EDT_COPY);
	}
}