#ifndef  __APPLOGIN_H
#define  __APPLOGIN_H

#include "TCPClient.h"
#include "IContainer.h"
#include "CommandData.h"

namespace app
{
	class AppLogin : public IContainer
	{
	public:
		AppLogin();
		virtual ~AppLogin();
	public:
		virtual void  onInit();
		virtual void  onUpdate();
		virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd);

		static void send_Login(FString name, int32 namelen, FString password, int32 passwordlen);
		static void send_Register(FString name, int32 namelen, FString password, int32 passwordlen);
		static void send_LoginGame(FString name, int32 namelen);
	};

	extern AppLogin* __AppLogin;
}


#endif
