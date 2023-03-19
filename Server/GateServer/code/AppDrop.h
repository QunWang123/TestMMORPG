#ifndef  ____APPDROP_H
#define  ____APPDROP_H

#include "IContainer.h"
namespace app
{
	class AppDrop : public IContainer
	{
	public:
		AppDrop();
		virtual ~AppDrop();
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);


	};

	extern IContainer* __AppDrop;
}

#endif