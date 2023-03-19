#ifndef  ____AppBag_H
#define  ____AppBag_H


#include "IContainer.h"
namespace app
{
	class AppBag : public IContainer
	{
	public:
		AppBag();
		virtual ~AppBag();
		virtual bool onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
	};

	extern IContainer* __AppBag;
}

#endif