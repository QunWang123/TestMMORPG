#include "AppGate.h"
#include <algorithm>
#include <time.h>
#include "AppGlobal.h"
#include "ShareFunction.h"

using namespace func;

namespace app
{
	AppGate* __AppGate = nullptr;

	std::map<u32, S_SERVER_MANAGER*>  __GateList;
	std::map<u32, S_SERVER_MANAGER*>  __DBList;
	std::map<u32, S_SERVER_MANAGER*>  __GameList;
	S_SERVER_MANAGER* findGateList(u32 clientID)
	{
		auto it = __GateList.find(clientID);
		if (it != __GateList.end())
		{
			return it->second;
		}
		return nullptr;
	}
	S_SERVER_MANAGER* findGameList(u32 clientID)
	{
		auto it = __GameList.find(clientID);
		if (it != __GameList.end())
		{
			return it->second;
		}
		return nullptr;
	}
	S_SERVER_MANAGER* findDBList(u32 clientID)
	{
		auto it = __DBList.find(clientID);
		if (it != __DBList.end())
		{
			return it->second;
		}
		return nullptr;
	}

	u32  temp_Time = 0;

	AppGate::AppGate()
	{
	}
	AppGate::~AppGate()
	{
	}

	void AppGate::onUpdate()
	{
		//5秒去申请一次数据 DB的数据
		s32 value = time(NULL) - temp_Time;
		if (value < 5) return;
		temp_Time = time(NULL);

		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE) 
		{
			auto d = findDBList(__TcpDB->getData()->serverID);
			if (d != nullptr)
			{
				d->state = 0;
			}
			return;
		}

		__TcpDB->begin(CMD_40);
		__TcpDB->end();
	}

	void AppGate::setGateListState(u32 clientID, u8 state)
	{
		auto g = findGateList(clientID);
		if (g != nullptr) g->state = state;
	}

	void outServerManager(S_SERVER_MANAGER* s)
	{
// 		char ftime[30];
// 		share::formatTime(time(0), ftime);
// 
// 		std::string str = "err";
// 		share::serverTypeToString(s->id, str);
// 
// 		f32 value = (f32)s->memory / (1024 * 1024);
// 		LOG_MESSAGE("%s [%sServer line:%d id:%d  state:%d] [ip:%s:%d] [online:%d] [memory:%.4f MB] \n",
// 			ftime, str.c_str(), s->line, s->id, s->state, s->ip, s->port, s->online, value);
	}
	//login gate，登录服务器发来的，说明有玩家在登录，把gate和game的一些数据发送给登录服务器（负载均衡）
	void onGetGateList(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u8 size = __GateList.size();
		u8 size2 = __GameList.size();

		ts->begin(c->ID, CMD_40);
		//1 发送网关列表负载均衡数据
		ts->sss(c->ID, size);
		auto it = __GateList.begin();
		while (it != __GateList.end())
		{
			auto d = it->second;
			if (d != nullptr)
			{
				ts->sss(c->ID, d, sizeof(S_SERVER_MANAGER));
			}
			++it;
		}
		//2 发送游戏服务器列表负载均衡数据
		ts->sss(c->ID, size2);
		auto it2 = __GameList.begin();
		while (it2 != __GameList.end())
		{
			auto d = it2->second;
			if (d != nullptr)
			{
				ts->sss(c->ID, d, sizeof(S_SERVER_MANAGER));
			}
			++it2;
		}
		ts->end(c->ID);

	}
	//gate gate
	void onPostGateList(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 id = 0;
		u8 line = 0; //线路 
		s32 online = 0;
		s32 memory = 0;
		
		ts->read(c->ID, id);
		ts->read(c->ID, line);
		ts->read(c->ID, online);
		ts->read(c->ID, memory);

		u32 type = share::serverIDToType(id);
		if (type == func::S_TYPE_GATE)
		{
			//更新在线人数
			S_SERVER_MANAGER* g = findGateList(id);
			if (g == nullptr)
			{
				LOG_MESSAGE("findGateList not gate:%d...\n", id);
				return;
			}
			g->line = line;
			g->online = online;
			g->memory = memory;
			outServerManager(g);
			return;
		}
		if (type == func::S_TYPE_GAME)
		{
			//更新在线人数
			S_SERVER_MANAGER* g = findGameList(id);
			if (g == nullptr)
			{
				LOG_MESSAGE("findGameList not game:%d...\n", id);
				return;
			}
			g->line = line;
			g->online = online;
			g->memory = memory;

			if (memory == 0)
			{
				g->state = 0;
			}
			else
			{
				g->state = c->state;
			}
			outServerManager(g);
			return;
		}
	}

	bool AppGate::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppLogin err...line:%d \n", __LINE__);
			return false;
		}
		if (cmd != CMD_40) return false;

		switch (c->clientType)
		{
		case func::S_TYPE_LOGIN:
			onGetGateList(ts, c);
			break;
		case func::S_TYPE_GATE:
			onPostGateList(ts, c);
			break;
		}
		
		return false;
	}


	//*********************************************************************
	//*********************************************************************
	void onCMD_40(net::ITCPClient* tc)
	{
		s32 id = 0;
		s32 online = 0;
		s32 memory = 0;
		tc->read(id);
		tc->read(online);
		tc->read(memory);
		
		//更新在线人数
		auto g = findDBList(id);
		if (g == nullptr)
		{
			LOG_MESSAGE("findDBList not db:%d...\n", id);
			return;
		}
		g->line = 0;
		g->online = online;
		g->memory = memory;
		g->state = tc->getData()->state;

		outServerManager(g);
		
	}
	bool AppGate::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_DB) return false;

		switch (cmd)
		{
		case CMD_40:  onCMD_40(tc);  break;
		}

		return false;
	}
}