#include "CommandData2.h"
#include "AppChat.h"
#include "UserManager.h"
#include "AppGlobal.h"

namespace app
{
	AppChat* __AppChat = nullptr;

	AppChat::AppChat()
	{
	}
	AppChat::~AppChat()
	{
	}
	void AppChat::onInit()
	{
	}

	//2000
	void onChat(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		char nick[USER_MAX_NICK];
		char text[50];

		ts->read(c->ID, nick, USER_MAX_NICK);
		ts->read(c->ID, text, 50);

		__TcpCenter->begin(CMD_2000);
		__TcpCenter->sss(nick, USER_MAX_NICK);
		__TcpCenter->sss(text, 50);
		__TcpCenter->end();
	}

	bool AppChat::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppEntryWorld err...line:%d \n", __LINE__);
			return false;
		}
		if (c->clientType != func::S_TYPE_USER) return false;
		if (c->state != func::S_LOGIN) return false;

		switch (cmd)
		{
		case CMD_2000:onChat(ts, c);  break;
		}
		return true;
	}


	//************************************************************************
	//************************************************************************
	//2000 ÁÄÌì
	void onCMD_2000(net::ITCPClient* tc)
	{
		u32 connectid;
		char nick[USER_MAX_NICK];
		char text[50];

		tc->read(connectid);
		tc->read(nick, USER_MAX_NICK);
		tc->read(text, 50);

		LOG_MESSAGE("onCMD_2000 ...%dline:%d \n", connectid, __LINE__);

		auto c = __TcpServer->client(connectid);
		if (c == NULL)
		{
			LOG_MESSAGE("onCMD_2000 c == NULL...%dline:%d \n", connectid, __LINE__);
			return;
		}
		
		__TcpServer->begin(c->ID, CMD_2000);
		__TcpServer->sss(c->ID, nick, USER_MAX_NICK);
		__TcpServer->sss(c->ID, text, 50);
		__TcpServer->end(c->ID);
	}

	bool AppChat::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		switch (cmd)
		{
		
		case CMD_2000:  onCMD_2000(tc);  break;

		}
		return false;
	}
}