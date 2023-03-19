#ifndef __APPSKILL_H
#define __APPSKILL_H

#include "IContainer.h"
#include "UDPClient.h"

namespace app
{
	class AppSkill : public IContainer
	{
	public:
		AppSkill();
		virtual ~AppSkill();
		virtual void  onInit();
		virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd);

		static void send_Skill(int32 skillindex, int lock_type, int lock_index, FVector targetpos);
		static void send_Reborn(int32 kind);

#ifdef TEST_UDP
		static  void onClientCommand_Udp(UDPClient* udp, int32 cmd);
#endif

	};

	extern AppSkill* __AppSkill;
}


#endif
