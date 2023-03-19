#ifndef __APPCHAT_H
#define __APPCHAT_H

#include "IContainer.h"

namespace app
{
	class AppChat : public IContainer
	{
	public:
		AppChat();
		virtual ~AppChat();
		virtual void  onInit();
		virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd);

		static void send_ChatText(FString text);
	};

	extern AppChat* __AppChat;
}


#endif


