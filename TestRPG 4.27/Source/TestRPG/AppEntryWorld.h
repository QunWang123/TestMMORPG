#ifndef __APPENTRYWORLD_H
#define __APPENTRYWORLD_H

#include "IContainer.h"

namespace app
{
	class AppEntryWorld : public IContainer
	{
	public:
		AppEntryWorld();
		virtual ~AppEntryWorld();
		virtual void  onInit();
		virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd);

		static void send_EntryWorld();
		static void send_GetOtherRoleData(int32 userindex);
		static void send_ChangeMap(int32 mapid);
		static void send_ChangeLine(uint8 line);
	};

	extern AppEntryWorld* __AppEntryWorld;
}


#endif

