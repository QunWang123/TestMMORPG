#include "AppCopy.h"
#include  "CommandData2.h"
#include  "UserManager.h"
// #include  "AppTeam.h"
namespace app
{

	AppCopy*  __AppCopy = nullptr;
	s32 temp_index = 0;
	s32 temp_checkTime = 0;
	// void onLeaveCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c);

	AppCopy::AppCopy(){ }
	AppCopy::~AppCopy(){}


	//7200 开启单人副本
//7300 开启多人副本
	void onStartCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		u32 mapid = 0;
		ts->read(c->ID, mapid);

		//1、验证玩家有数据 1
		S_USER_GATE* user = __UserManager->findUser(c->ID, c->memid);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, cmd, 4001);
			return;
		}
		s32 ftime = time(NULL) - user->temp_GetCopyTime;
		if (ftime < USER_COPY_TIME)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 4002);
			return;
		}
		user->temp_GetCopyTime = time(NULL);
		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 4003);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 4004);
			return;
		}


		S_CMD_COPY data;
		data.user_connectid = c->ID;
		data.memid = user->memid;
		data.userindex = user->userindex;
		data.mapid = mapid;

		//发送给游戏服务器
		tcp->begin(cmd);
		tcp->sss((u16)0);
		tcp->sss(&data, sizeof(S_CMD_COPY));
		tcp->end();
	}

	bool AppCopy::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppCopy err...line:%d \n", __LINE__);
			return false;
		}
		if (c->clientType != func::S_TYPE_USER) return false;

		switch (cmd)
		{
		case CMD_7200://单人副本
		case CMD_7300://多人副本
		case CMD_7400://离开副本
			onStartCopy(ts, c,cmd);
			break;
		}
		return false;
	}

	

	
	//**************************************************************************
	//**************************************************************************
	void onCMD_7200(net::ITCPClient* tc, const u16 cmd)
	{
		u16 childcmd = 0;
		S_CMD_COPY data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_CMD_COPY));


		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL || user->connection == NULL)
		{
			LOG_MESSAGE("onCMD_ user == NULL...%d %d-%lld line:%d \n",cmd, data.user_connectid, data.memid, __LINE__);
			return;
		}
		LOG_MESSAGE("copy cmd:%d err:%d  memid:%lld\n", cmd, childcmd,data.memid);

		if (childcmd != 0 && childcmd != 3000)
		{
			sendErrInfo(__TcpServer, data.user_connectid, cmd, childcmd);
			return;
		}
		//切换成功
		if (childcmd == 0)
		{
			char pos[12];
			tc->read(pos, 12);
			auto c = user->connection;

			LOG_MESSAGE("send copy cmd:%d err:%d  memid:%lld cid:%d\n", cmd, childcmd, data.memid,c->ID);

			__TcpServer->begin(c->ID, cmd);
			__TcpServer->sss(c->ID, (u16)0);
			__TcpServer->sss(c->ID, data.mapid);
			__TcpServer->sss(c->ID, pos, 12);
			__TcpServer->end(c->ID);
			return;
		}

		net::ITCPClient* newmapTcp = __UserManager->findGameServer_Connection(user->line, data.mapid);
		if (newmapTcp == nullptr)
		{
			sendErrInfo(__TcpServer, data.user_connectid, cmd, 4002);
			return;
		}
		if (newmapTcp->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, data.user_connectid, cmd, 4003);
			return;
		}

		//设置新地图ID和新的连接
		user->tcpGame = newmapTcp;
		user->mapid = data.mapid;

		user->tcpGame->begin(cmd);
		user->tcpGame->sss((u16)1);
		user->tcpGame->sss(&data, sizeof(S_CMD_COPY));
		user->tcpGame->end();
	}
	void onCMD_7301(net::ITCPClient* tc, const u16 cmd)
	{
		//S_CMD_COPY data;
		//tc->read(&data, sizeof(S_CMD_COPY));

		//auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		//if (user == NULL)
		//{
		//	LOG_MSG("onCMD_ user == NULL...%d %d-%lld line:%d \n", cmd, data.user_connectid, data.memid, __LINE__);
		//	return;
		//}
		//auto c = user->connection;
		//__TcpServer->begin(c->ID, CMD_7301);
		//__TcpServer->sss(c->ID, data.mapid);
		//__TcpServer->end(c->ID);
	}
	bool AppCopy::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_GAME && tc->getData()->serverType != func::S_TYPE_CENTER) return false;

		switch (cmd)
		{
		case CMD_7200:
		case CMD_7300:
		case CMD_7400:
		case CMD_7301:
			onCMD_7200(tc, cmd);
			break;
		//case CMD_7301://收到开始进入副本的消息
		//	onCMD_7301(tc, cmd);
		//	break;
		}

		return true;
	}
}


