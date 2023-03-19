#include "AppChat.h"
#include "CommandData.h"
#include "MyGameInstance.h"
#include "BlueprintFunction.h"
#include "AppManager.h"

namespace app
{
	AppChat* __AppChat = nullptr;
	AppChat::AppChat()
	{

	}

	AppChat::~AppChat()
	{

	}

	void AppChat::onInit()
	{

	}

	void onCMD_2000(net::TCPClient* tc)
	{
		if (__AppGameInstance != NULL) __AppGameInstance->onEntryWorldCommand(CMD_2000, -1, -1);
	}

	bool AppChat::onClientCommand(net::TCPClient* tc, const uint16 cmd)
	{
		switch (cmd)
		{
		case CMD_2000:
			onCMD_2000(tc);
			break;
		}
		return true;
	}

	void AppChat::send_ChatText(FString text)
	{
		__TcpClient->begin(CMD_2000);
		UBlueprintFunction::send_FString_len(__myUserData.rolebase.nick, USER_MAX_NICK);
		UBlueprintFunction::send_FString_len(text, 50);
		__TcpClient->end();
	}
}

