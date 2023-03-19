#include "AppMove.h"
#include "CommandData.h"
#include "UserManager.h"
#include "AppGlobal.h"
namespace app
{
	AppMove* __AppMove = nullptr;

	AppMove::AppMove()
	{

	}

	AppMove::~AppMove()
	{

	}

	void AppMove::onInit()
	{

	}

	void onMove(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_MOVE_ROLE move;

		ts->read(c->ID, move.face);
		ts->read(c->ID, move.speed);

		ts->read(c->ID, &move.curpos, sizeof(S_VECTOR3));
		ts->read(c->ID, &move.targetpos, sizeof(S_VECTOR3));

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_1000, 4001);
			return;
		}

		move.user_connectid = c->ID;
		move.userindex = user->userindex;
		move.memid = user->memid;

		if (user->tcpGame == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_1000, 4002);
			return;
		}
		if (user->tcpGame->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_1000, 4003);
			return;
		}

		user->tcpGame->begin(CMD_1000);
		user->tcpGame->sss(&move, sizeof(S_MOVE_ROLE));
		user->tcpGame->end();
	}

	bool AppMove::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppMove err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_USER || c->state != func::S_LOGIN) return false;

		switch (cmd)
		{
		case CMD_1000:
			onMove(ts, c);
			break;
		}


		return true;
	}

	void onCMD_1000(net::ITCPClient* tc)
	{
		u16 childcmd;
		S_MOVE_ROLE move;

		tc->read(childcmd);
		tc->read(&move, sizeof(S_MOVE_ROLE));

		auto user = __UserManager->findUser(move.user_connectid, move.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("onCMD_1000 err user == NULL...%d-%lld line:%d",
				move.user_connectid, move.memid, __LINE__);
			return;
		}

		// 3000 是拉回上一个坐标
		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_1000);
		__TcpServer->sss(c->ID, childcmd);
		if (childcmd == 3000)
		{
			LOG_MESSAGE("back to pos %d-%d-%d\n", move.targetpos.x, move.targetpos.y, move.targetpos.z);
			__TcpServer->sss(c->ID, &move.targetpos, sizeof(S_VECTOR3));
		}
		__TcpServer->end(c->ID);
	}

	void onCMD_1100(net::ITCPClient* tc)
	{
		S_MOVE_ROLE move;
		tc->read(&move, sizeof(S_MOVE_ROLE));

		auto user = __UserManager->findUser(move.user_connectid, move.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("onCMD_1100 err user == NULL...%d-%lld line:%d",
				move.user_connectid, move.memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_1100);
		__TcpServer->sss(c->ID, move.userindex);
		__TcpServer->sss(c->ID, move.face);
		__TcpServer->sss(c->ID, move.speed);
		__TcpServer->sss(c->ID, &move.targetpos, sizeof(S_VECTOR3));
		__TcpServer->end(c->ID);
	}

	bool AppMove::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_GAME) return false;

		switch (cmd)
		{
		case CMD_1000:
			onCMD_1000(tc);		// 自己移动错误，没错误的话不会走这里的
			break;
		case CMD_1100:
			onCMD_1100(tc);		// 其他玩家收到xx玩家的移动数据
			break;
		}


		return true;
	}

}