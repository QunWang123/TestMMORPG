#ifndef  ____APPDROP_H
#define  ____APPDROP_H

#include "IContainer.h"
namespace app
{
	class AppDrop : public IContainer
	{
	public:
		AppDrop();
		virtual ~AppDrop();
		virtual void  onInit();
		virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd);
		static void send_GetDrop(int32 dropindex, int32 dropid);
	};

	extern IContainer* __AppDrop;
}

#endif