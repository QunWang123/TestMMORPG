#include "AppEnterWorld.h"
#include "CommandData.h"
#include "WorldData.h"
#include "UserManager.h"
#include "AppGlobal.h"
namespace app
{
	AppEnterWorld* __AppEntry = nullptr;

	AppEnterWorld::AppEnterWorld()
	{

	}

	AppEnterWorld::~AppEnterWorld()
	{

	}

	void AppEnterWorld::onInit()
	{

	}
	// CMD_700
	void onGetOtherRoleData(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_GETROLE_DATA data;
		ts->read(c->ID, data.targetindex);
		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_700, 4001);
			return;
		}
		data.user_connectid = c->ID;
		data.memid = user->memid;
		
		if (user->tcpGame == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_700, 4002);
			return;
		}
		if (user->tcpGame->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_700, 4003);
			return;
		}

		net::ITCPClient* tcp = user->tcpGame;
		tcp->begin(CMD_700);
		tcp->sss(&data, sizeof(S_GETROLE_DATA));
		tcp->end();
	}

	void onEntryWorld(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_900, 4001);
			return;
		}
		if (user->tcpGame == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_900, 4002);
			return;
		}
		if (user->tcpGame->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_900, 4003);
			return;
		}
		// user->userindex是玩家在游戏服务器上的ID，同时也是在DB上的ID，是由DB赋值后才传给游戏服务器、中心服务器的
		// CMD_200赋值的
		S_ENTRY_BASE data;
		data.userindex = user->userindex;
		data.memid = user->memid;
		data.user_connectid = c->ID;

		// 发送给玩家对应的游戏服务器
		net::ITCPClient* tcp = user->tcpGame;
		tcp->begin(CMD_900);
		tcp->sss(&data, sizeof(S_ENTRY_BASE));
		tcp->end();
	}

	//901 切换地图
	void onChangeMap(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 mapid = 0;
		ts->read(c->ID, mapid);

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_901, 4001);
			return;
		}
		net::ITCPClient* newmapTcp = __UserManager->findGameServer_Connection(user->line, mapid);
		if (user->tcpGame == nullptr || newmapTcp == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_901, 4002);
			return;
		}
		if (user->tcpGame->getData()->state < func::C_CONNECT_SECURE || newmapTcp->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_901, 4003);
			return;
		}
		//控制下 切换地图时间  3秒内 操作 返回操作过快错误
		//这个时间可以自己调整
		s32 ftime = time(NULL) - user->temp_GetChangeMapTime;
		if (ftime < USER_GETCHANGEMAP_TIME)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_901, 4004);
			return;
		}
		user->temp_GetChangeMapTime = time(NULL);

		S_CHANGEMAP_BASE data;
		data.userindex = user->userindex;
		data.memid = c->memid;
		data.user_connectid = c->ID;
		data.mapid = mapid;
		data.line = user->line;

		user->tcpGame->begin(CMD_901);
		user->tcpGame->sss((u16)0);
		user->tcpGame->sss(&data, sizeof(S_CHANGEMAP_BASE));
		user->tcpGame->end();
	}
	//902 切换线路
	void onChangeLine(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u8 line = 0;
		ts->read(c->ID, line);

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_902, 4001);
			return;
		}
		net::ITCPClient* newmapTcp = __UserManager->findGameServer_Connection(line, user->mapid);
		if (user->tcpGame == nullptr || newmapTcp == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_902, 4002);
			return;
		}
		if (user->tcpGame->getData()->state < func::C_CONNECT_SECURE || newmapTcp->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_902, 4003);
			return;
		}
		//控制下 切换地图时间  3秒内 操作 返回操作过快错误
		//这个时间可以自己调整
		//切换线路和切换地图 共用一个时间控制
		s32 ftime = time(NULL) - user->temp_GetChangeMapTime;
		if (ftime < USER_GETCHANGEMAP_TIME)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_902, 4004);
			return;
		}
		user->temp_GetChangeMapTime = time(NULL);

		S_CHANGEMAP_BASE data;
		data.userindex = user->userindex;
		data.memid = c->memid;
		data.user_connectid = c->ID;
		data.mapid = user->mapid;
		data.line = line;

		user->tcpGame->begin(CMD_902);
		user->tcpGame->sss((u16)0);
		user->tcpGame->sss(&data, sizeof(S_CHANGEMAP_BASE));
		user->tcpGame->end();
	}

	bool AppEnterWorld::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppEnterWorld err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_USER || c->state != func::S_LOGIN) return false;

		switch (cmd)
		{
		case CMD_700:
			onGetOtherRoleData(ts, c);
			break;
		case CMD_900:
			onEntryWorld(ts, c);
			break;
		case CMD_901:
			onChangeMap(ts, c);  
			break;  //切换地图
		case CMD_902:
			onChangeLine(ts, c); 
			break; //切换线路
		}
		return true;
	}

	// 某位玩家掉线通知
	void onCMD_600(net::ITCPClient* tc)
	{
		u32 connectid;
		u64 memid;
		u32 userindex;			// 掉线玩家下标索引

		tc->read(connectid);
		tc->read(memid);
		tc->read(userindex);
		
		auto user = __UserManager->findUser(connectid, memid);
		if (user == NULL)
		{
			// sendErrInfo(__TcpServer, connectid, CMD_600, 4001);
			LOG_MESSAGE("onCMD_600 user == null...%d-%lld line:%d\n", connectid, memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_600);
		__TcpServer->sss(c->ID, userindex);
		__TcpServer->end(c->ID);

	}

	//602 DB与game服务器断开
	void onCMD_602(net::ITCPClient* tc)
	{
		u32 userindex = 0;//掉线玩家的 下标索引
		u64 memid = 0;

		tc->read(userindex);
		tc->read(memid);
		auto user = __UserManager->findUser_Connection(userindex, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppEntryWorld user == NULL...%d-%lldd line:%d \n", userindex, memid, __LINE__);
			return;
		}

		LOG_MESSAGE("closeClient DB disconnect Game...%lld-%d\n", memid, userindex);
		// io::pushLog(io::EFT_ERR, "closeClient DB disconnect Game...%lld-%d\n", memid, userindex);

		auto c = user->connection;
		if (c == nullptr) return;
		__TcpServer->closeClient(c->ID);
	}

	// 获取某位玩家数据
	void onCMD_700(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_GETROLE_DATA data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_GETROLE_DATA));

		auto user = __UserManager->findUser(data.user_connectid, data.memid);
		if (user == NULL)
		{
			// sendErrInfo(__TcpServer, connectid, CMD_600, 4001);
			LOG_MESSAGE("onCMD_700 user == null...%d-%lld line:%d\n", 
				data.user_connectid, data.memid, __LINE__);
			return;
		}

		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, data.user_connectid, CMD_700, childcmd);
			return;
		}

		S_ROLE_BASE_INNATE			innate;		// 先天属性
		S_ROLE_BASE_STATUS			status;		// 状态
		S_ROLE_BASE_EXP				exp;		// 经验
		S_ROLE_BASE_LIFE			life;		// 生命

		tc->read(&innate, C_ROLE_INNATE_LEN);
		tc->read(&status, C_ROLE_STATUS_LEN);
		tc->read(&exp, C_ROLE_EXP_LEN);
		tc->read(&life, sizeof(S_ROLE_BASE_LIFE));

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_700);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, data.targetindex);
		__TcpServer->sss(c->ID, &innate, C_ROLE_INNATE_LEN);
		__TcpServer->sss(c->ID, status.face);
		__TcpServer->sss(c->ID, &status.pos, 12);
		__TcpServer->sss(c->ID, exp.level);
		__TcpServer->sss(c->ID, life.hp);
		__TcpServer->end(c->ID);
	}

	// 更新玩家生命
	void onCMD_710(net::ITCPClient* tc, u16 cmd)
	{
		S_UPDATE_VALUE data;
		tc->read(&data, sizeof(S_UPDATE_VALUE));
		auto user = __UserManager->findUser(data.user_connectid, data.memid);
		if (user == NULL)
		{
			// sendErrInfo(__TcpServer, connectid, CMD_600, 4001);
			LOG_MESSAGE("onCMD_710 user == null...%d-%lld line:%d\n", 
				data.user_connectid, data.memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, cmd);
		__TcpServer->sss(c->ID, data.targetindex);
		if (cmd == CMD_730)
		{
			__TcpServer->sss(c->ID, (u8)data.value);
		}
		else 
		{
			__TcpServer->sss(c->ID, data.value);
		}
		__TcpServer->end(c->ID);
	}

	//740 更新经验 只发给自己
	void onCMD_740(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		u32 exp = 0;
		tc->read(connectid);
		tc->read(memid);
		tc->read(exp);

		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("onCMD_740 user == NULL...%d-%lldd line:%d \n", connectid, memid, __LINE__);
			return;
		}
		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_740);
		__TcpServer->sss(c->ID, exp);
		__TcpServer->end(c->ID);
	}

	//750 升级 广播所有人
	void onCMD_750(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		u32 userindex = 0;
		u16 level = 0;
		tc->read(connectid);
		tc->read(memid);
		tc->read(userindex);
		tc->read(level);
		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("onCMD_740 user == NULL...%d-%lldd line:%d \n", connectid, memid, __LINE__);
			return;
		}
		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_750);
		__TcpServer->sss(c->ID, userindex);
		__TcpServer->sss(c->ID, level);
		__TcpServer->end(c->ID);
	}

	void onCMD_760(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		auto user = __UserManager->findUser(connectid, memid);
		if (user == NULL)
		{
			// sendErrInfo(__TcpServer, connectid, CMD_600, 4001);
			LOG_MESSAGE("onCMD_760 user == null...%d-%lld line:%d\n",
				connectid, memid, __LINE__);
			return;
		}
		u32 gold = 0;
		u32 diamonds = 0;

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_760);
		__TcpServer->sss(c->ID, gold);
		__TcpServer->sss(c->ID, diamonds);
		__TcpServer->end(c->ID);
	}

	void onCMD_770(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		char atk[44];
		tc->read(connectid);
		tc->read(memid);
		tc->read(&atk, 44);
		auto user = __UserManager->findUser(connectid, memid);
		if (user == NULL)
		{
			// sendErrInfo(__TcpServer, connectid, CMD_600, 4001);
			LOG_MESSAGE("onCMD_770 user == null...%d-%lld line:%d\n",
				connectid, memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_770);
		__TcpServer->sss(c->ID, atk, 44);
		__TcpServer->end(c->ID);
	}

	void onCMD_900(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_ENTRY_BASE data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_ENTRY_BASE));

		auto user = __UserManager->findUser(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("onCMD_900 err user == NULL...%d-%lld line:%d\n", 
				data.user_connectid, data.memid, __LINE__);
			return;
		}

		sendErrInfo(__TcpServer, data.user_connectid, CMD_900, childcmd);
	}

	//901 切换地图
	void onCMD_901(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_CHANGEMAP_BASE data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_CHANGEMAP_BASE));

		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("onCMD_901 user == NULL...%d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
		if (childcmd != 0 && childcmd != 3000)
		{
			sendErrInfo(__TcpServer, data.user_connectid, CMD_901, childcmd);
			return;
		}
		//切换成功
		if (childcmd == 0)
		{
			char pos[12];
			tc->read(pos, 12);
			auto c = user->connection;
			__TcpServer->begin(c->ID, CMD_901);
			__TcpServer->sss(c->ID, (u16)0);
			__TcpServer->sss(c->ID, data.mapid);
			__TcpServer->sss(c->ID, pos, 12);
			__TcpServer->end(c->ID);
			return;
		}

		net::ITCPClient* newmapTcp = __UserManager->findGameServer_Connection(user->line, data.mapid);
		if (newmapTcp == nullptr)
		{
			sendErrInfo(__TcpServer, data.user_connectid, CMD_901, 4002);
			return;
		}
		if (newmapTcp->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, data.user_connectid, CMD_901, 4003);
			return;
		}

		//设置新地图ID和新的连接
		user->tcpGame = newmapTcp;
		user->mapid = data.mapid;

		user->tcpGame->begin(CMD_901);
		user->tcpGame->sss((u16)1);
		user->tcpGame->sss(&data, sizeof(S_CHANGEMAP_BASE));
		user->tcpGame->end();
	}
	//902 切换line
	void onCMD_902(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_CHANGEMAP_BASE data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_CHANGEMAP_BASE));

		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("onCMD_902 user == NULL...%d-%lldd line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
		if (childcmd != 0 && childcmd != 3000)
		{
			sendErrInfo(__TcpServer, data.user_connectid, CMD_902, childcmd);
			return;
		}
		//切换成功
		if (childcmd == 0)
		{
			auto c = user->connection;
			__TcpServer->begin(c->ID, CMD_902);
			__TcpServer->sss(c->ID, (u16)0);
			__TcpServer->end(c->ID);
			return;
		}

		net::ITCPClient* newmapTcp = __UserManager->findGameServer_Connection(data.line, data.mapid);
		if (newmapTcp == nullptr)
		{
			sendErrInfo(__TcpServer, data.user_connectid, CMD_902, 4002);
			return;
		}
		if (newmapTcp->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, data.user_connectid, CMD_902, 4003);
			return;
		}

		//设置新线路和新的连接
		user->tcpGame = newmapTcp;
		user->line = data.line;

		user->tcpGame->begin(CMD_902);
		user->tcpGame->sss((u16)1);
		user->tcpGame->sss(&data, sizeof(S_CHANGEMAP_BASE));
		user->tcpGame->end();
	}

	bool AppEnterWorld::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		switch (cmd)
		{
		case CMD_600:
			onCMD_600(tc);
			break;
		case CMD_602:
			onCMD_602(tc);
			break;
		case CMD_700:
			onCMD_700(tc);
			break;
		case CMD_710:
		case CMD_720:
		case CMD_730:
			onCMD_710(tc, cmd);
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

}



