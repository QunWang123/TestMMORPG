#include "AppGate.h"
#include <algorithm>
#include <time.h>
#include "AppGlobal.h"


namespace app
{

	AppGate* __AppGate = nullptr;
	std::vector<S_SERVER_MANAGER*> __GateList;
	std::vector<S_SERVER_MANAGER*> __GameList;
	S_GAME_LINE  __GameLine[MAX_LINE_COUNT];

	u32  temp_Time = 0;

	AppGate::AppGate()
	{
		
	}
	AppGate::~AppGate()
	{
	}
	
	void AppGate::onUpdate()
	{
		//5秒去申请一次数据
		s32 value = time(NULL) - temp_Time;
		if (value < 5) return;
		temp_Time = time(NULL);

		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE) return;
		__TcpCenter->begin(CMD_40);
		__TcpCenter->end();
	}

	void initGateOnline(u8 size)
	{
		LOG_MESSAGE("init gateList...\n");

		if (__GateList.size() > 0) return;

		for (u32 i = 0; i < size; i++)
		{
			S_SERVER_MANAGER* s = new S_SERVER_MANAGER();
			s->reset();
			__GateList.push_back(s);
		}
	}

	void initGameOnline(u8 size)
	{
		LOG_MESSAGE("init gameList...\n");

		if (__GameList.size() > 0) return;

		for (u32 i = 0; i < size; i++)
		{
			S_SERVER_MANAGER* s = new S_SERVER_MANAGER();
			s->reset();
			__GameList.push_back(s);
		}

	}

	bool gateOnlineSort(S_SERVER_MANAGER* a, S_SERVER_MANAGER* b)
	{
		if (a == nullptr || b == nullptr) return false;

		if (a->state > b->state)          return true;
		else if (a->state < b->state)     return false;

		if (a->online < b->online)        return true;
		else if (a->online > b->online)   return false;

		return false;
	}

	void onCMD_40(net::ITCPClient* tc)
	{
		//1 读取网关的负载均衡数据
		u8 size = 0;
		tc->read(size);

		if(__GateList.size() == 0) initGateOnline(size);

		for (u32 i = 0; i < size; i++)
		{
			auto g = __GateList[i];
			if (g == nullptr) continue;
			tc->read(g,sizeof(S_SERVER_MANAGER));
		}
		// 对__GateList进行排序，使其__GateList[0]位置总是最优的
		std::sort(__GateList.begin(), __GateList.end(), gateOnlineSort);

		//2 读取游戏服务器的负载均衡数据
		u8 size2 = 0;
		tc->read(size2);
		if (__GameList.size() == 0) initGameOnline(size2);

		memset(__GameLine, 0, sizeof(S_GAME_LINE) * MAX_LINE_COUNT);

		for (u32 i = 0; i < size2; i++)
		{
			auto g = __GameList[i];
			if (g == nullptr) continue;
			tc->read(g, sizeof(S_SERVER_MANAGER));

			if (g->line >= MAX_LINE_COUNT) continue;
			// 一条gameline会有许多个gameserver
			__GameLine[g->line].isuse = 1;
			__GameLine[g->line].line = g->line;
			__GameLine[g->line].count += g->online;
		}

		//3 输出打印
		char ftime[30];
		share::formatTime(time(0), ftime);

		LOG_MESSAGE("******************************************** \n");
		LOG_MESSAGE("%s \n", ftime);
		for (u32 i = 0; i < size; i++)
		{
			auto g = __GateList[i];
			if (g == nullptr) continue;
			if (g->memory == 0)
			{
				LOG_MESSAGE("未连接 gateserver id:%d ip:%s-%d  \n",
					g->id, g->ip, g->port);
				continue;
			}
			f32 m = (f32)g->memory / (1024 * 1024);
			LOG_MESSAGE("[GateServer:%d] [id:%d] [ip:%s-%d] [online:%d] [memory:%.2f MB] \n",
				i,  g->id, g->ip, g->port, g->online, m);
		}

		for (u32 i = 0; i < size2; i++)
		{
			auto g = __GameList[i];
			if (g == nullptr) continue;

			if (g->memory == 0)
			{
				LOG_MESSAGE("未连接gameserver id:%d ip:%s-%d  \n",
					 g->id, g->ip, g->port);
				continue;
			}
			f32 m = (f32)g->memory / (1024 * 1024);
			LOG_MESSAGE("[GameServer:%d] [id:%d] [ip:%s-%d] [online:%d] [memory:%.2f MB] \n",
				g->line, g->id, g->ip,g->port, g->online,m);
		}

		for (u32 i = 0; i < MAX_LINE_COUNT; i++)
		{
			auto& g = __GameLine[i];
			if (g.isuse == 0) continue;
			LOG_MESSAGE("line:%d count:%d \n",i, g.count);
		}
	}
	bool AppGate::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_CENTER) return false;

		switch (cmd)
		{
		case CMD_40:  onCMD_40(tc);  break;
		}

		return false;
	}
}