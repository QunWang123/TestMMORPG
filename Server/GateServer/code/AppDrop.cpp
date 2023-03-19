
#include  "AppDrop.h"
#include  "UserManager.h"
#include "CommandData.h"
#include "AppGlobal.h"
namespace app
{
	IContainer* __AppDrop;
	u32  checkmapid = 0;
	s64  checktime = 0;

	AppDrop::AppDrop() {}
	AppDrop::~AppDrop() {}

	void onGetDrop(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_DROP_DATA  data;
		ts->read(c->ID, data.dropindex);
		ts->read(c->ID, data.dropid);

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_6000, 4001);
			return;
		}
		//LOG_MSG("move...%lld---%d/%d\n", c->memid, move.targetpos.x, move.targetpos.y);

		data.user_connectid = c->ID;
		data.userindex = user->userindex;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_6000, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_6000, 4003);
			return;
		}

		//发送给游戏服务器
		tcp->begin(CMD_6000);
		tcp->sss(&data, sizeof(S_DROP_DATA));
		tcp->end();

	}

	bool AppDrop::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppLogin err...line:%d \n", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_USER) return false;
		if (c->state != func::S_LOGIN) return false;

		switch (cmd)
		{
		case CMD_6000:onGetDrop(ts, c);  break;
		}

		return false;
	}





	//***********************************************************************
	// 错误
	void onCMD_6000(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		u16 err = 0;
		tc->read(connectid);
		tc->read(memid);
		tc->read(err);
		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppDrop user == NULL...%d-%lld line:%d \n", connectid, memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_6000);
		__TcpServer->sss(c->ID, err);
		__TcpServer->end(c->ID);

	}
	// 正常发送掉落信息
	void onCMD_6100(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		char data[28];
		tc->read(connectid);
		tc->read(memid);
		tc->read(data, 28);
		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppDrop user == NULL...%d-%lld line:%d \n", connectid, memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_6100);
		__TcpServer->sss(c->ID, data, 28);
		__TcpServer->end(c->ID);

	}
	// 删除掉落
	void onCMD_6200(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		u32 dropindex = 0;
		tc->read(connectid);
		tc->read(memid);
		tc->read(dropindex);
		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppDrop user == NULL...%d-%lld line:%d \n", connectid, memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_6200);
		__TcpServer->sss(c->ID, dropindex);
		__TcpServer->end(c->ID);

	}


	bool AppDrop::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_GAME) return false;

		switch (cmd)
		{
		case CMD_6000:  onCMD_6000(tc);  break;
		case CMD_6100:  onCMD_6100(tc);  break;//
		case CMD_6200:  onCMD_6200(tc);  break;//
		}
		return false;
	}
}