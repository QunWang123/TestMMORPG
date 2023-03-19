#ifndef __APPSELECTROLE_H
#define __APPSELECTROLE_H

#include "TCPClient.h"
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
		virtual void  onUpdate();
		virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd);

		static void send_CreateRole(uint8 injob, uint8 insex, FString innick);
		static void send_DeleteRole(uint8 roleindex);
		static void send_StartGame(uint8 roleindex);
	};

	extern AppSelectRole* __AppSelectRole;
}

#endif
