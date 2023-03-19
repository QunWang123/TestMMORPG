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
		// virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);
	};

	extern AppSkill* __AppSkill;
}


#endif