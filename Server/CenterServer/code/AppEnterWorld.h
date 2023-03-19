#ifndef  ____APPENTERWORLD_H
#define  ____APPENTERWORLD_H

#include "IContainer.h"

namespace app
{
	class AppEnterWorld : public IContainer
	{
	public:
		AppEnterWorld();
		virtual ~AppEnterWorld();
	public:
		virtual void  onInit();
		//virtual void  onUpdate() {}
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		// virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);
	};

	extern AppEnterWorld* __AppEnterWorld;
}


#endif