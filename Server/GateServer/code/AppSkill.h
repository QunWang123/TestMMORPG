#ifndef  ____APPSKILL_H
#define  ____APPSKILL_H

#include "IContainer.h"

namespace app
{
	class AppSkill : public IContainer
	{
	public:
		AppSkill();
		virtual ~AppSkill();
	public:
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);
	};

	extern AppSkill* __AppSkill;
#ifdef  TEST_UDP_SERVER
	class AppSkill_Udp : public IUDPContainer
	{
	public:
		AppSkill_Udp();
		virtual ~AppSkill_Udp();
		virtual bool  onUDPServerCommand(net::IUdpServer* udp, net::S_UDP_BASE* c, const u16 cmd);

	};

	extern void onCMD_3100_Udp(net::ITCPClient* tc);
	extern AppSkill_Udp* __AppSkilludp;
#endif

}


#endif