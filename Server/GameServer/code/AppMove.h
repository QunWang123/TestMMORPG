#ifndef  ____APPMOVE_H
#define  ____APPMOVE_H

#include "IContainer.h"

namespace app
{
	class AppMove : public IContainer
	{
	public:
		AppMove();
		virtual ~AppMove();
	public:
		virtual void  onInit();
		//virtual void  onUpdate() {}
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
	};

	extern AppMove* __AppMove;
}


#endif