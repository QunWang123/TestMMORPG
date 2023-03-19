#ifndef  ____APPGATE_H
#define  ____APPGATE_H 

#include "IContainer.h"
#include "CommandData.h"
#include <ShareFunction.h>
namespace app
{
	struct S_GAME_LINE
	{
		u8  isuse;
		u8  line;
		s32 count;
	};

	class AppGate : public IContainer
	{
	public:
		AppGate();
		virtual ~AppGate();
		virtual void  onUpdate();
	
		virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);
		
	};

	extern AppGate* __AppGate;
	extern S_GAME_LINE  __GameLine[MAX_LINE_COUNT];
	extern std::vector<S_SERVER_MANAGER*> __GateList;
	extern std::vector<S_SERVER_MANAGER*> __GameList;
}

#endif