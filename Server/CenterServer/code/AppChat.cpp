#include "AppChat.h"
#include "CommandData.h"
#include "WorldData.h"
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

	void onChat(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		char nick[USER_MAX_NICK];
		char text[50];

		ts->read(c->ID, nick, USER_MAX_NICK);
		ts->read(c->ID, text, 50);

		// 应该还有工会聊天什么什么的，这里姑且不做区分
		int count = func::__ServerInfo->MaxUser;

		for (int i = 0; i < count; i++)
		{
			auto user = __UserManager->findUser(i);
			if (user == nullptr || !user->mem.isT()) continue;
			if (user->mem.state < M_LOGINING)
			{
				LOG_MESSAGE("char err %d-%d-%d\n", 
					user->mem.state, user->tmp.server_connectid, user->tmp.user_connectid);
				continue;
			}
			__TcpServer->begin(user->tmp.server_connectid, CMD_2000);
			__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
			__TcpServer->sss(user->tmp.server_connectid, nick, USER_MAX_NICK);
			__TcpServer->sss(user->tmp.server_connectid, text, 50);
			__TcpServer->end(user->tmp.server_connectid);
		}
	}

	bool AppChat::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppEnterWorld err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_2000:
			onChat(ts, c);
			break;
		}
		return true;
	}

// 	void onCMD_2000(net::ITCPClient* tc)
// 	{
// 		u32 connectid;
// 		char nick[USER_MAX_NICK];
// 		char text[50];
// 
// 		tc->read(connectid);
// 		tc->read(nick, USER_MAX_NICK);
// 		tc->read(text, 50);
// 		LOG_MESSAGE("onCMD_2000 ... %d  line:%d \n", connectid, __LINE__);
// 
// 		auto c = __TcpServer->client(connectid);
// 
// 		if (c == NULL)
// 		{
// 			LOG_MESSAGE("onCMD_2000 c == NULL ... %d  line:%d \n",
// 				connectid, __LINE__);
// 		}
// 
// 		__TcpServer->begin(c->ID, CMD_2000);
// 		__TcpServer->sss(c->ID, nick, USER_MAX_NICK);
// 		__TcpServer->sss(c->ID, text, 50);
// 		__TcpServer->end(c->ID);
// 	}
// 
// 	bool AppEnterWorld::onClientCommand(net::ITCPClient* tc, const u16 cmd)
// 	{
// 		switch (cmd)
// 		{
// 		case CMD_2000:
// 			onCMD_2000(tc);
// 			break;
// 		}
// 		return true;
// 	}
}



