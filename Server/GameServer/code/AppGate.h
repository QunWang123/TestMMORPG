#ifndef  ____APPGATE_H
#define  ____APPGATE_H 

#include "IContainer.h"
#include "CommandData.h"
namespace app
{

	class AppGate : public IContainer
	{
	public:
		AppGate();
		virtual ~AppGate();
		virtual void  onUpdate();
		virtual bool onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);

	};

	extern AppGate* __AppGate;
}

#endif