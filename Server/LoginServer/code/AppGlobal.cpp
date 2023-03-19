#include "AppGlobal.h"
#include "ShareFunction.h"

namespace app
{
	net::ITCPServer* __TcpServer = nullptr;
	net::ITCPClient* __TcpCenter = nullptr;
	http::HttpClient* __HttpClient[MAX_HTTP_THREAD];

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
		int aa = 0;
		int bb = 0;
		tcp->getSecurityCount(aa, bb);
		func::setConsoleColor(10);
		LOG_MESSAGE("security connect... %d [%s:%d][connect:%d-%d] [ClientID:%d-%s]\n", (int)c->socketfd, c->ip, c->port, aa, bb, c->clientID, type.c_str());
		func::setConsoleColor(7);
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
		LOG_MESSAGE("disconnect-%d [%s::%s][con:%d-%d] [ClientID:%d-%s]\n", (int)c->socketfd, str1, str2, aa, bb, c->clientID, type.c_str());
		func::setConsoleColor(7);

		if (c->state == func::S_CONNECT || c->state == func::S_CONNECT_SECURE)
		{
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
		LOG_MESSAGE("-------client security connect...[%s-%d] [ClientID:%d-%s]\n", tcp->getData()->ip, tcp->getData()->port, tcp->getData()->serverID, type.c_str());
		func::setConsoleColor(7);
	}
	//工作线程OR主线程
	extern void onDisConnect(net::ITCPClient* tcp, const s32 code)
	{
		std::string type;
		share::serverTypeToString(tcp->getData()->serverID, type);
		func::setConsoleColor(8);
		LOG_MESSAGE("-------client onDisconnect...[%s-%d] [ClientID:%d-%s]\n", tcp->getData()->ip, tcp->getData()->port, tcp->getData()->serverID, type.c_str());
		func::setConsoleColor(7);
	}
	//主线程
	void onExceptConnect(net::ITCPClient* tcp, const s32 code)
	{
		LOG_MESSAGE("-------client onOnExcept...%d\n", code);
	}

}

