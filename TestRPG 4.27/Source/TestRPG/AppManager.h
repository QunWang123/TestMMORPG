#ifndef __APPMANAGER_H
#define __APPMANAGER_H

#include "TCPClient.h"

namespace app
{
	class AppManager
	{
	public:
		AppManager();
		~AppManager();

		void Init();
	};
	extern int	run();
	extern void	onUpdate();
	extern net::TCPClient* __TcpClient;
}

#endif