#ifndef  ____APPGLOBAL_H
#define  ____APPGLOBAL_H


#include "INetBase.h"
#include "IContainer.h"
#include <time.h>
#include <map>

namespace app
{
	extern net::ITCPServer* __TcpServer;
	extern net::ITCPClient* __TcpCenter;
	extern std::vector<net::ITCPClient*> __TcpGame;
	//事件API
	extern void onClientAccept(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void onClientSecureConnect(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void onClientDisConnect(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void onClientTimeout(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void onClientExcept(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);

	extern void onConnect(net::ITCPClient* tcp, const s32 code);
	extern void onSecureConnect(net::ITCPClient* tcp, const s32 code);
	extern void onExceptConnect(net::ITCPClient* tcp, const s32 code);
	extern void onDisConnect(net::ITCPClient* tcp, const s32 code);

	extern void sendErrInfo(net::ITCPServer* ts, const s32 connectID, const u16 cmd, const u16 childcmd);

#ifdef TEST_UDP_SERVER
	extern net::IUdpServer* __IUdpServer;
	//事件API
	extern void  onUdpToClientAccept(net::IUdpServer* udo, net::S_UDP_BASE* c, const s32 code, const char* err);
	extern void  onUdpToClientDisconnect(net::IUdpServer* udp, net::S_UDP_BASE* c, const s32 code, const char* err);
#endif

}



#endif