#ifndef  ____APPROBOT_H
#define  ____APPROBOT_H

#include "IContainer.h"
#include "CommandData.h"

namespace app
{
	class AppRobot : public IContainer
	{
	public:
		AppRobot();
		virtual ~AppRobot();
	public:
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
	};
	extern AppRobot* __AppRobot;
}

#endif
