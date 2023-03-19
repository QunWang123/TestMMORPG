#ifndef  ____APPCOPY_H
#define  ____APPCOPY_H

#include  "APPGlobal.h"
#include "WorldData.h"
#include "CommandData2.h"
namespace app
{
	class  AppCopy :public IContainer
	{
	public:
		AppCopy();
		virtual ~AppCopy();
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);

	};


	extern AppCopy*  __AppCopy;
}
#endif