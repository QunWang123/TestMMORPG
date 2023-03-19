#ifndef  ____APPGLOBAL_H
#define  ____APPGLOBAL_H


#include "INetBase.h"
#include "IContainer.h"
#include <time.h>
#include <map>
#include "HttpServer.h"

namespace app
{
	extern net::ITCPServer* __TcpServer;
	extern net::ITCPServer* __TcpCenter;
	extern http::HttpServer*  __HttpServer;
	

	//�¼�API
	extern void onClientAccept(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void onClientSecureConnect(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void onClientDisConnect(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void onClientTimeout(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void onClientExcept(net::ITCPServer* tcp, net::S_CLIENT_BASE* c, const s32 code);

	extern void onConnect(net::ITCPClient* tcp, const s32 code);
	extern void onSecureConnect(net::ITCPClient* tcp, const s32 code);
	extern void onExceptConnect(net::ITCPClient* tcp, const s32 code);
	extern void onDisConnect(net::ITCPClient* tcp, const s32 code);


}



#endif