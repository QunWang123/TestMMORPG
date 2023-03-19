#include "AppLogin.h"
#include "AppManager.h"
#include "BlueprintFunction.h"
#include "UserData.h"
#include "MyGameInstance.h"


namespace app
{
	AppLogin* __AppLogin = nullptr;


	AppLogin::AppLogin()
	{

	}

	AppLogin::~AppLogin()
	{

	}

	void AppLogin::onInit()
	{

	}

	void AppLogin::onUpdate()
	{

	}

	void AppLogin::send_Login(FString name, int32 namelen, FString password, int32 passwordlen)
	{
		app::__TcpClient->begin(CMD_10);
		UBlueprintFunction::send_FString_len(name, namelen);
		UBlueprintFunction::send_FString_len(password, passwordlen);
		app::__TcpClient->end();
	}

	void AppLogin::send_Register(FString name, int32 namelen, FString password, int32 passwordlen)
	{
		app::__TcpClient->begin(CMD_20);
		UBlueprintFunction::send_FString_len(name, namelen);
		UBlueprintFunction::send_FString_len(password, passwordlen);
		app::__TcpClient->end();
	}

	// 100 登录游戏
	void AppLogin::send_LoginGame(FString name, int32 namelen)
	{
		app::__TcpClient->begin(CMD_100);
		UBlueprintFunction::send_FString_len(name, namelen);
		app::__TcpClient->sss(loginkey, USER_MAX_KEY);
		app::__TcpClient->sss((uint8)0);
		app::__TcpClient->end();
	}

	void onLogin(net::TCPClient* tcp)
	{
		uint16 childcmd = 0;
		tcp->read(childcmd);

		uint8  ip[20];
		uint16 port = 0;

		if (childcmd == 0)
		{
			tcp->read(&loginkey, USER_MAX_KEY);
			tcp->read(ip, 20);
			tcp->read(port);
		}

		if (__AppGameInstance != nullptr)
		{
			__AppGameInstance->onLoginCommand(CMD_10, childcmd);
		}
		
		if (childcmd != 0)
		{
			FString ss = FString::Printf(TEXT("login err:%d"), childcmd);
			if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20, FColor::Emerald, ss);

			return;
		}
		FString gate_fip = UTF8_TO_TCHAR(ip);

		// 登录成功，切换ip和端口号
		app::__TcpClient->resetIP(gate_fip, port);
		app::__TcpClient->disconnectServer(9000, "onLongin change gate server");

	}

	void onRegister(net::TCPClient* tcp)
	{
		uint16 childcmd = 0;
		tcp->read(childcmd);
		if (__AppGameInstance != nullptr)
		{
			__AppGameInstance->onLoginCommand(CMD_20, childcmd);
		}
	}

	// 进入游戏获取角色的基础数据
	void onRoleBaseData(net::TCPClient* tcp)
	{
		uint16 childcmd = 0;
		tcp->read(childcmd);
		if (childcmd == 0)
		{
			tcp->read(__myUserData.memid);
			for (int i = 0; i < USER_MAX_ROLE; i++)
			{
				tcp->read(&rolebase[i], 18);
				uint16 level = 0;
				tcp->read(level);
				rolebase[i].level = level;
				rolebase[i].nick = UBlueprintFunction::read_FString_len(USER_MAX_NICK);
			}
		}
		else
		{
			for (int i = 0; i < USER_MAX_ROLE; i++)
			{
				FMemory::Memset(&rolebase[i], 0, 22);
				rolebase[i].nick = "";
			}
		}
		if (__AppGameInstance != nullptr) __AppGameInstance->onLoginCommand(CMD_100, childcmd);
	}

	bool AppLogin::onClientCommand(net::TCPClient* tc, const uint16 cmd)
	{
		switch (cmd)
		{
		case CMD_10:
			onLogin(tc);
			break;
		case CMD_20:
			onRegister(tc);
			break;
		case CMD_100:
			onRoleBaseData(tc);
			break;
		}
		
		return true;
	}
}
