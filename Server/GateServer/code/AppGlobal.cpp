#include "AppGlobal.h"
#include "ShareFunction.h"
#include "CommandData.h"
#include "UserManager.h"


namespace app
{
	net::ITCPServer* __TcpServer = nullptr;
	net::ITCPClient* __TcpCenter = nullptr;
	std::vector<net::ITCPClient*> __TcpGame;
	//工作线程
	void onClientAccept(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)   return;

		LOG_MESSAGE("new connect...%d [%s-%d] %d\n", (int)c->socketfd, c->ip, c->port,c->threadID);
	}
	//主线程
	void onClientSecureConnect(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)   return;
		std::string type;
		share::serverTypeToString(c->clientID, type);

		c->temp_LoginTime = 0;

		int aa = 0;
		int bb = 0;
		tcp->getSecurityCount(aa, bb);
		func::setConsoleColor(10);
		LOG_MESSAGE("security connect... %d [%s:%d][connect:%d-%d] [ClientID:%d-%s]\n", 
			(int)c->socketfd, c->ip, c->port, aa, bb, c->clientID, type.c_str());
		func::setConsoleColor(7);
		c->temp_LoginTime = time(NULL);
	}
	//主线程
	void onClientDisConnect(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)   return;
		std::string type;
		share::serverTypeToString(c->clientID, type);

		int aa = 0;
		int bb = 0;
		tcp->getSecurityCount(aa, bb);
		const char* str1 = func::getShutDownError(c->shutdown_kind);
		const char* str2 = func::getCloseSocketError(code);

		func::setConsoleColor(8);
		LOG_MESSAGE("disconnect-%d [%s::%s][con:%d-%d] [ClientID:%d-%s]\n",
			(int)c->socketfd, str1, str2, aa, bb, c->clientID, type.c_str());
		func::setConsoleColor(7);


		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);
		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);

#ifdef TEST_UDP_SERVER
		//清理UDP连接
		auto udp = __IUdpServer->findClient(c->ID);
		if (udp != NULL)
		{
			udp->memid = 0;
			__IUdpServer->clearConnect(udp);

			func::setConsoleColor(8);
			LOG_MESSAGE("udp tcp_disconnect %d\n", c->ID);
			func::setConsoleColor(7);
		}
#endif

		if (c->state == func::S_CONNECT)
		{
			c->Reset();
		}
		else if (c->state == func::S_CONNECT_SECURE)
		{
			if (c->memid > 0)
			{
				__UserManager->sendWorld_UserLeaveWorld(c->memid);
				auto user = __UserManager->findUser(c->ID, c->memid);
				if (user != NULL) user->reset();
			}
			c->Reset();
			LOG_MESSAGE("AppGlobal secure leave %d-%d\n", c->ID, (int)c->socketfd);
		}
		else if (c->state == func::S_LOGIN)
		{
			auto user = __UserManager->findUser(c->ID, c->memid);
			if (user != NULL)
			{
// 				io::pushLog(io::EFT_ERR, "disconnect-%d [%s::%s][con:%d-%d] [clientid:%d-%s] [memory:%.4f/%.4f] err:%d %lld %s\n",
// 					(int)c->socketfd, str1, str2, aa, bb, c->clientID, type.c_str(), curmemory1, maxmemory1, code, c->memid, user->name);
				__UserManager->sendWorld_UserLeaveWorld(user);
				user->reset();
			}

			LOG_MESSAGE("AppGlobal login leave...%d-%d \n", c->ID, (int)c->socketfd);
			c->Reset();
		}
	}
	//主线程
	void onClientTimeout(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 cmd)
	{
		if (c == nullptr || tcp == nullptr) return;

	}

	void onClientExcept(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 cmd)
	{
		if (c == nullptr || tcp == nullptr) return;
	}


	//客户端
	////////////////
    //工作线程
	extern void onConnect(net::ITCPClient* tcp, const s32 code)
	{
		//LOG_MESSAGE("-------client connect...%d %d\n", code, tcp->getData()->ID);
	}
	//主线程
	extern void onSecureConnect(net::ITCPClient* tcp, const s32 code)
	{
		std::string type;
		share::serverTypeToString(tcp->getData()->serverID, type);
		func::setConsoleColor(13);
		LOG_MESSAGE("-------client security connect...[%s-%d] [ClientID:%d-%s]\n", 
			tcp->getData()->ip, tcp->getData()->port, tcp->getData()->serverID, type.c_str());
		func::setConsoleColor(7);

		// 这里是gateserver作为客户端新连了一个服务端（比如聊天服务器等等），则需要将gateserver上的所有玩家
		// 都发送过去
		u32 server_type = share::serverIDToType(tcp->getData()->ID);
		switch (server_type)
		{
		case func::S_TYPE_CENTER:
			if (__UserManager) __UserManager->sendWorld_UserEntryWorld();
			break;
		}
	}
	//工作线程OR主线程
	extern void onDisConnect(net::ITCPClient* tcp, const s32 code)
	{
		std::string type;

		share::serverTypeToString(tcp->getData()->serverID, type);
		func::setConsoleColor(8);
		LOG_MESSAGE("-------client onDisconnect...%d [%s-%d] [ClientID:%d-%s]\n", 
			code, tcp->getData()->ip, tcp->getData()->port, tcp->getData()->serverID, type.c_str());

		func::setConsoleColor(7);

		u8 stype = share::serverIDToType(tcp->getData()->serverID);
		if (__UserManager != nullptr) __UserManager->setTcpClientDisconnect(stype);
	}
	//主线程
	void onExceptConnect(net::ITCPClient* tcp, const s32 code)
	{
		LOG_MESSAGE("-------client onOnExcept...%d\n", code);
	}

	void sendErrInfo(net::ITCPServer* ts, const s32 connectID, const u16 cmd, const u16 childcmd)
	{
		ts->begin(connectID, cmd);
		ts->sss(connectID, childcmd);
		ts->end(connectID);
	}

#ifdef TEST_UDP_SERVER
	net::IUdpServer* __IUdpServer = nullptr;
	//维护新的连接
	void onUdpToClientAccept(net::IUdpServer* udp, net::S_UDP_BASE* c, const s32 code, const char* err)
	{
		char s[30];
		func::formatTime(time(NULL), s);

		func::setConsoleColor(10);
		LOG_MESSAGE("udp connect %s...id:%d [%s:%d] \n", s, c->ID, c->strip, c->port);
		func::setConsoleColor(7);
	}
	//用户失去连接
	//1 心跳包判断玩家掉线
	//2 10054 
	//3 客户端显示调用65003 离开
	void onUdpToClientDisconnect(net::IUdpServer* udp, net::S_UDP_BASE* c, const s32 code, const char* err)
	{
		char s[30];
		func::formatTime(time(NULL), s);

		func::setConsoleColor(8);
		LOG_MESSAGE("udp disconnect %s...id:%d [%s:%d] err:%d/%s\n", s, c->ID, c->strip, c->port, code, err);
		func::setConsoleColor(7);

		//onPlayerLeave(udp, c);
		// udp->clearConnect(c);
	}
#endif
}

