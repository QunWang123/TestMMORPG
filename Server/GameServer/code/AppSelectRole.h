#ifndef  ____APPSELECTROLE_H
#define  ____APPSELECTROLE_H

#include "IContainer.h"
#include "CommandData.h"

namespace app
{
	class AppSelectRole : public IContainer
	{
	public:
		AppSelectRole();
		virtual ~AppSelectRole();
	public:
		virtual void  onInit();
		//virtual void  onUpdate() {}
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);
	};
	extern AppSelectRole* __AppSelectRole;
}

#endif
