#ifndef __APPROBOT_H
#define __APPROBOT_H

#include "IContainer.h"

namespace app
{
	class AppRobot : public IContainer
	{
	public:
		AppRobot();
		virtual ~AppRobot();
		virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd);

		static void send_RobotData(int32 robotindex);
	};

	extern AppRobot* __AppRobot;
}


#endif