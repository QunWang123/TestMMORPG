#ifndef  ____APPCHAT_H
#define  ____APPCHAT_H

#include "IContainer.h"

namespace app
{
	class AppChat : public IContainer
	{
	public:
		AppChat();
		virtual ~AppChat();
	public:
		virtual void  onInit();
		//virtual void  onUpdate() {}
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		// virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);
	};

	extern AppChat* __AppChat;
}


#endif