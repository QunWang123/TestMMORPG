#include "AppManager.h"
#include "MyGameInstance.h"
#include "CommandData.h"
#include "AppLogin.h"
#include "AppSelectRole.h"
#include "AppEntryWorld.h"
#include "AppMove.h"
#include "UserData.h"
#include "WorldScript.h"
#include "AppSkill.h"
#include "AppRobot.h"
#include "AppBag.h"
#include "AppDrop.h"
#include "AppChat.h"
#include "UDPClient.h"

namespace app
{
	AppManager* __AppManager = nullptr;
	net::TCPClient* __TcpClient = nullptr;

	AppManager::AppManager(){}

	AppManager::~AppManager(){}

	void onUpdate()
	{
		if (__TcpClient == nullptr) return;

		__TcpClient->parseCommand();
		renderUserEvent();
		renderUserBuff();
		updateSkillColdTime();

		renderRobotEvent();
		renderRobotBuff();
		updateRobotData();		// 看看是否在玩家视野内

#ifdef TEST_UDP
		if (__TcpClient->getData()->state >= func::C_CONNECT_SECURE)
		{
			if (__udp->getData()->port != app::login_port && __udp != NULL)
				__udp->parseCommand();
		}
#endif
	}

	//**********************************************************
	void onConnect(net::TCPClient* tcp, const int32 code)
	{
		__AppGameInstance->onConnect(code);
	}

	void onSecureConnect(net::TCPClient* tcp, const int32 code)
	{
#ifdef TEST_UDP
		tcp->read(func::__UDPClientInfo->ID);
		if (tcp->getData()->port == app::login_port) return;

		int32 port = tcp->getData()->port + 100;
		__udp->resetIP(tcp->getData()->ip, port);
#endif
		__AppGameInstance->onSecureConnect(code);
	}

	void onDisConnect(net::TCPClient* tcp, const int32 code)
	{
#ifdef TEST_UDP
		if (__udp) __udp->clearConnect();
#endif
		__AppGameInstance->onDisConnect(code);
	}

	void onExceptConnect(net::TCPClient* tcp, const int32 code)
	{
		__AppGameInstance->onExceptConnect(code);
	}

	

	void onCommand(net::TCPClient* tcp, const int32 cmd)
	{
		switch (cmd)
		{
		case CMD_10:
		case CMD_20:
		case CMD_100:
			__AppLogin->onClientCommand(tcp, cmd);
			break;
		case CMD_200:
		case CMD_300:
		case CMD_400:
		case CMD_500:
			__AppSelectRole->onClientCommand(tcp, cmd);
			break;
		case CMD_600:
		case CMD_700:
		case CMD_710:
		case CMD_720:
		case CMD_730:
		case CMD_740:
		case CMD_750:
		case CMD_760:
		case CMD_770:
		case CMD_900:
		case CMD_901:
		case CMD_902:
			__AppEntryWorld->onClientCommand(tcp, cmd);
			break;
		case CMD_800:
		case CMD_810:
		case CMD_820:
		case CMD_830:
		case CMD_840:
		case CMD_850:
		case CMD_860:
		case CMD_870:
			__AppBag->onClientCommand(tcp, cmd);
			break;
		case CMD_1000:
		case CMD_1100:
			__AppMove->onClientCommand(tcp, cmd);
			break;
		case CMD_2000:
			__AppChat->onClientCommand(tcp, cmd);
		case CMD_3000:
		case CMD_3100:
		case CMD_3200:
		case CMD_4000:
		case CMD_4100:
		case CMD_4200:
		case CMD_5000:
			__AppSkill->onClientCommand(tcp, cmd);
			break;
		case CMD_6000:
		case CMD_6100:
		case CMD_6200:
			__AppDrop->onClientCommand(tcp, cmd);
			break;
		case CMD_8000:
		case CMD_8100:
		case CMD_8200:
		case CMD_8300:
		case CMD_8400:
		case CMD_8500:
		case CMD_8700:
		case CMD_8800:
			__AppRobot->onClientCommand(tcp, cmd);
			break;
		default:
			if (__AppGameInstance) __AppGameInstance->onCommand(cmd);
			break;
		}

		// __AppGameInstance->onCommand(code);
	}



	void InitClientXML()
	{
		if (func::__ClientInfo == nullptr)
		{
			func::__ClientInfo = new func::ConfigXML();
		}
		// 正式项目的话，这些需要从远程下载下来	
		func::__ClientInfo->SafeCode = "yifeimm";
		func::__ClientInfo->Head[0] = 'D';
		func::__ClientInfo->Head[1] = 'E';
		func::__ClientInfo->RCode = 130;
		func::__ClientInfo->Version = 20180408;
		func::__ClientInfo->RecvOne = 8 * 1024;
		func::__ClientInfo->RecvMax = 64 * 1024;
		func::__ClientInfo->SendOne = 8 * 1024;
		func::__ClientInfo->SendMax = 64 * 1024;
		func::__ClientInfo->HeartTime = 15;		// 15秒没收到就算断开连接
		func::__ClientInfo->AutoTime = 1;		// 3秒自动发一次
	}

	//**********************************************************
#ifdef TEST_UDP
	void InitClientXML_Udp()
	{
		if (func::__UDPClientInfo == nullptr) func::__UDPClientInfo = new func::ConfigXML();

		func::__UDPClientInfo->Head[0] = 'D';
		func::__UDPClientInfo->Head[1] = 'E';
		func::__UDPClientInfo->RCode = 130;
		func::__UDPClientInfo->Version = 20210803;
		func::__UDPClientInfo->RecvOne = 8 * 1024;
		func::__UDPClientInfo->RecvMax = 256 * 1024;
		func::__UDPClientInfo->SendOne = 8 * 1024;
		func::__UDPClientInfo->SendMax = 256 * 1024;
		func::__UDPClientInfo->HeartTime = 3 * 1000;
		func::__UDPClientInfo->HeartTimeMax = 30 * 1000;
		func::__UDPClientInfo->AutoTime = 1 * 1000;
	}

	void onConnect_Udp(UDPClient* udp, int32 cmd, FString err)
	{
		FString ss = FString::Printf(TEXT("udp connect success...%d id:%d"), cmd, func::__UDPClientInfo->ID);
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20, FColor::Emerald, ss);

		//__AppGameInstance->onUDPConnect(cmd);
	}
	void onDisConnect_Udp(UDPClient* udp, int32 cmd, FString err)
	{
		FString ss = FString::Printf(TEXT("udp disconnect...%d id:%d"), cmd, func::__UDPClientInfo->ID);
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20, FColor::Emerald, ss);

		//__AppGameInstance->onUDPDisconnect(cmd);
		udp->clearConnect();
	}
	void onCommand_Udp(UDPClient* udp, int32 cmd, FString err)
	{
		AppSkill::onClientCommand_Udp(udp, cmd);
		//__AppTest->onUDPClientCommand(udp, cmd);
		//__AppPlayer->onUDPClientCommand(udp, cmd);
	}
#endif

	void AppManager::Init()
	{
		script::initScript();
		InitClientXML();
		InitMapInfo(1);
		
		

#ifdef TEST_UDP
		InitClientXML_Udp();
		__udp = new UDPClient();
		__udp->setOnUdpServerAccept(onConnect_Udp);
		__udp->setOnUdpServerDisconnect(onDisConnect_Udp);
		__udp->setOnUdpServerCommand(onCommand_Udp);
		__udp->runClient("127.0.0.1", 15001);
#endif

		__TcpClient = new net::TCPClient();
		__TcpClient->setOnConnect(onConnect);
		__TcpClient->setOnDisConnect(onDisConnect);
		__TcpClient->setOnExcept(onExceptConnect);
		__TcpClient->setOnSecureConnect(onSecureConnect);
		__TcpClient->setOnCommand(onCommand);
		__TcpClient->runClient(0, app::login_ip, app::login_port);
		__TcpClient->getData()->ID = 0;	

		__AppLogin = new AppLogin();
		__AppSelectRole = new AppSelectRole();
		__AppEntryWorld = new AppEntryWorld();
		__AppMove = new AppMove();
		__AppSkill = new AppSkill();
		__AppRobot = new AppRobot();
		__AppDrop = new AppDrop();
		__AppBag = new AppBag();
		__AppChat = new AppChat();
	}

	int run()
	{
		if (__AppManager == nullptr)
		{
			__AppManager = new AppManager();
			__AppManager->Init();
		}

		app::__TcpClient->setThread(false);
		return 0;
	}
}


