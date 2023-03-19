#ifndef  ____APPLOGIN_H
#define  ____APPLOGIN_H

#include "IContainer.h"
#include "CommandData.h"

namespace app
{
	class AppLogin : public IContainer
	{
	public:
		AppLogin();
		virtual ~AppLogin();
	public:
		virtual void  onInit();
		//virtual void  onUpdate() {}
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);
	};
	extern AppLogin* __AppLogin;
}

#endif

