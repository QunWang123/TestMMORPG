#ifndef  ____APPGAME_H
#define  ____APPGAME_H 

#include "IContainer.h"
#include "CommandData.h"
namespace app
{

	class AppGame : public IContainer
	{
	public:
		AppGame();
		virtual ~AppGame();
		virtual void  onInit();
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  onDBCommand(void* buff, const u16 cmd);
	};

	extern AppGame* __AppGame;
}

#endif