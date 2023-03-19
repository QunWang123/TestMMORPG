#include "AppRobot.h"
#include "WorldData.h"
#include "UserManager.h"
#include "AppGlobal.h"
namespace app
{
	AppRobot* __AppRobot = nullptr;
	AppRobot::AppRobot()
	{

	}

	AppRobot::~AppRobot()
	{

	}

	void AppRobot::onInit()
	{

	}

	void onData(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_ROBOT_DATA data;
		ts->read(c->ID, data.robotindex);

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_8000, 4001);
			return;
		}

		data.user_connectid = c->ID;
		data.memid = c->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_8000, 4002);
			return;
		}
		if (tcp->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_8000, 4003);
			return;
		}
		LOG_MESSAGE("send AppRobot 8000  ... %d\n", data.robotindex);
		tcp->begin(CMD_8000);
		tcp->sss(&data, sizeof(S_ROBOT_DATA));
		tcp->end();
	}

	bool AppRobot::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppMove err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_USER || c->state != func::S_LOGIN) return false;

		switch (cmd)
		{
		case CMD_8000:
			onData(ts, c);
			break;
		}
		return true;
	}

	void onCMD_8000(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_ROBOT_DATA data;
		
		tc->read(childcmd);
		tc->read(&data, sizeof(S_ROBOT_DATA));

		S_USER_GATE* user = __UserManager->findUser(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppRobot_8000 err ... user == null %d-%lld %d\n",
				data.user_connectid, data.memid, __LINE__);
			return;
		}
		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_8000, childcmd);
			return;
		}

		char robot[17];
		tc->read(robot, 17);

		__TcpServer->begin(c->ID, CMD_8000);
		__TcpServer->sss(c->ID, childcmd);
		__TcpServer->sss(c->ID, data.robotindex);
		__TcpServer->sss(c->ID, robot, 17);
		__TcpServer->end(c->ID);
	}

	void onCMD_8100(net::ITCPClient* tc, const u16 cmd)
	{
		S_ROBOT_DATA data;
		tc->read(&data, sizeof(S_ROBOT_DATA));

		S_USER_GATE* user = __UserManager->findUser(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppRobot_%d err ... user == null %d-%lld %d\n",
				cmd, data.user_connectid, data.memid, __LINE__);
			return;
		}
		auto c = user->connection;
		__TcpServer->begin(c->ID, cmd);
		__TcpServer->sss(c->ID, data.robotindex);
		switch (cmd)
		{
		case CMD_8100:

			break;
		case CMD_8200:
		{
			s32 hp = 0;
			tc->read(hp);
			__TcpServer->sss(c->ID, hp);
		}
			break;
		case CMD_8300:
		{
			u8 state = 0;
			tc->read(state);
			__TcpServer->sss(c->ID, state);
		}
			break;
		case CMD_8400:
		{
			char g[8];
			tc->read(&g, 8);
			__TcpServer->sss(c->ID, g, 8);
		}
			break;
		case CMD_8500:
		{
			char g[10];
			tc->read(&g, 10);
			__TcpServer->sss(c->ID, g, 10);
		}
			break;
		case CMD_8700:
		{
			char g[13];
			tc->read(&g, 13);
			__TcpServer->sss(c->ID, g, 13);
		}
			break;
		}
		__TcpServer->end(c->ID);
	}

	void onCMD_8800(net::ITCPClient* tc)
	{
		s32 connectid = 0;
		s64 memid = 0;
		s32 buff_robotindex = 0;
		s32 buff_id = 0;
		s32 buff_runningtime = 0;

		tc->read(connectid);
		tc->read(memid);
		tc->read(buff_robotindex);
		tc->read(buff_id);
		tc->read(buff_runningtime);
		
		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppRobot user == NULL... %d-%lld, line:%d\n", connectid, memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_8800);
		__TcpServer->sss(c->ID, buff_robotindex);
		__TcpServer->sss(c->ID, buff_id);
		__TcpServer->sss(c->ID, buff_runningtime);
		__TcpServer->end(c->ID);
	}

	bool AppRobot::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_GAME) return false;
		switch (cmd)
		{
		case CMD_8000:
			onCMD_8000(tc);
			break;
		case CMD_8100:
		case CMD_8200:
		case CMD_8300:
		case CMD_8400:
		case CMD_8500:
		case CMD_8700:
			onCMD_8100(tc, cmd);
			break;
		case CMD_8800:
			onCMD_8800(tc);
			break;
		}
	}

}