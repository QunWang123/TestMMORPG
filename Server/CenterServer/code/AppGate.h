#ifndef  ____APPGATE_H
#define  ____APPGATE_H 

#include "IContainer.h"
#include "CommandData.h"
#include <map>
namespace app
{

	class AppGate : public IContainer
	{
	public:
		AppGate();
		virtual ~AppGate();
		virtual void  onUpdate();
		virtual bool onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);
	public:
		void setGateListState(u32 clientID, u8 state);
	};

	extern AppGate* __AppGate;
	extern std::map<u32, S_SERVER_MANAGER*>  __GateList;
	extern std::map<u32, S_SERVER_MANAGER*>  __DBList;
	extern std::map<u32, S_SERVER_MANAGER*>  __GameList;
}

#endif