#include "AppManager.h"
#include "AppGlobal.h"
#include "../../share/ShareFunction.h"

#ifndef  ___WIN32_
#include <unistd.h>
#endif
#include "UserManager.h"
#include "CommandData.h"
#include "AppLogin.h"
#include "AppSelectRole.h"
#include "AppEnterWorld.h"
#include "AppMove.h"
#include "AppSkill.h"
#include "AppRobot.h"
#include "AppGate.h"
#include "AppDrop.h"
#include "AppBag.h"
#include "AppChat.h"
#include "AppCopy.h"

#define TESTCONNECT 1

namespace app
{
	AppManager* __AppManager = nullptr;
	int temp_time = 0;
	char printfstr[1000];

	AppManager::AppManager()
	{
	}

	AppManager::~AppManager()
	{
	}


	//打印信息
	void printInfo()
	{
#ifdef  ___WIN32_
		int tempTime = (int)time(NULL) - temp_time;
		if (tempTime < 1) return;

		int connectcount = 0;
		int secureconnectcount = 0;
		__TcpServer->getSecurityCount(connectcount, secureconnectcount);

		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);
		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);

		int concount2 = 0;

#ifdef TEST_UDP_SERVER
		if (__IUdpServer) concount2 = __IUdpServer->getConnectCount();
#endif

		sprintf_s(printfstr, "GateServer [%d-%d] connect-%d, secureconnect-%d [memory:%.4f/%.4f] udp:%d", 
			func::__ServerInfo->ID, func::__ServerInfo->Port, connectcount, secureconnectcount, 
			curmemory1, maxmemory1, concount2);
		SetWindowTextA(GetConsoleWindow(), printfstr);
#endif
	}
	void onUpdate()
	{
		if (__TcpServer == nullptr) return;

		__TcpServer->parseCommand();
		__TcpCenter->parseCommand();
		int len = __TcpGame.size();
		for (int i = 0; i < len; i++)
		{
			__TcpGame[i]->parseCommand();
		}

		if (__AppGate != nullptr) __AppGate->onUpdate();
		if (__UserManager != nullptr) __UserManager->update();

		printInfo();
#ifdef TEST_UDP_SERVER
		if (__IUdpServer != nullptr) __IUdpServer->parseCommand();
#endif
	}

	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		if (func::__ServerListInfo.size() < 1) return;


		//创建服务器 启动
		__TcpServer = net::NewTcpServer();
		__TcpServer->setOnClientAccept(onClientAccept);
		__TcpServer->setOnClientSecureConnect(onClientSecureConnect);
		__TcpServer->setOnClientDisConnect(onClientDisConnect);
		__TcpServer->setOnClientTimeOut(onClientTimeout);
		__TcpServer->setOnClientExcept(onClientExcept);
		__TcpServer->runServer(1);
		// __TcpServer->runServer(3);		开3个能支持1W左右玩家登录
		// __TcpServer->registerCommand(CMD_REIGSTER, __AppPlayer);
		int len = func::__ServerListInfo.size();
		__TcpGame.reserve(len);
		for (int i = 0; i < len; i++)
		{
			auto xml = func::__ServerListInfo[i];
			auto client = net::NewTcpClient();
			client->setOnConnect(onConnect);
			client->setOnDisConnect(onDisConnect);
			client->setOnExcept(onExceptConnect);
			client->setOnSecureConnect(onSecureConnect);

			client->runClient(xml->ID, xml->IP, xml->Port);
			client->getData()->ID = i;


			u32 type = share::serverIDToType(xml->ID);
			switch (type)
			{
			case func::S_TYPE_CENTER:
				__TcpCenter = client;
				break;
			default:
				__TcpGame.emplace_back(client);
				break;
			}
		}
		
 		__UserManager = new UserManager();
 		__AppLogin = new AppLogin();
		__AppSelectRole = new AppSelectRole();
		__AppEntry = new AppEnterWorld();
		__AppMove = new AppMove();
		__AppSkill = new AppSkill();
		__AppRobot = new AppRobot();
		__AppGate = new AppGate();
		__AppDrop = new AppDrop();
		__AppBag = new AppBag();
		__AppChat = new AppChat();
		__AppCopy = new AppCopy();

#ifdef  TEST_UDP_SERVER
		__IUdpServer = net::NewUdpServer();
		__IUdpServer->setOnUdpClientAccept(onUdpToClientAccept);
		__IUdpServer->setOnUdpClientDisconnect(onUdpToClientDisconnect);
		__IUdpServer->runServer();

		__AppSkilludp = new AppSkill_Udp();
		__IUdpServer->registerCommand(CMD_3000, __AppSkilludp);//使用技能
#endif //  TEST_UDP_SERVER
 
 		__TcpServer->registerCommands(CMD_HEART, __AppLogin);
		__TcpServer->registerCommands(CMD_100, __AppLogin);			// 作为服务器时受到玩家发来的CMD_100
		__TcpServer->registerCommands(CMD_200, __AppSelectRole);	// 选择角色，正式进入游戏
		__TcpServer->registerCommands(CMD_300, __AppSelectRole);	// 创建角色
		__TcpServer->registerCommands(CMD_400, __AppSelectRole);	// 删除角色
		__TcpServer->registerCommands(CMD_700, __AppEntry);			// 获取其他角色数据
		__TcpServer->registerCommands(CMD_900, __AppEntry);			// 进入世界
		__TcpServer->registerCommands(CMD_901, __AppEntry);			// 切图
		__TcpServer->registerCommands(CMD_902, __AppEntry);			// 切线路

		__TcpServer->registerCommands(CMD_820, __AppBag);
		__TcpServer->registerCommands(CMD_830, __AppBag);
		__TcpServer->registerCommands(CMD_840, __AppBag);
		__TcpServer->registerCommands(CMD_850, __AppBag);
		__TcpServer->registerCommands(CMD_860, __AppBag);
		__TcpServer->registerCommands(CMD_870, __AppBag);
		__TcpServer->registerCommands(CMD_890, __AppBag);

		__TcpServer->registerCommands(CMD_1000, __AppMove);
		__TcpServer->registerCommands(CMD_2000, __AppChat);
		__TcpServer->registerCommands(CMD_3000, __AppSkill);
		__TcpServer->registerCommands(CMD_4100, __AppSkill);
		__TcpServer->registerCommands(CMD_6000, __AppDrop);
		__TcpServer->registerCommands(CMD_8000, __AppRobot);

		__TcpServer->registerCommands(CMD_7200, __AppCopy);
		__TcpServer->registerCommands(CMD_7300, __AppCopy);
		__TcpServer->registerCommands(CMD_7400, __AppCopy);

		__TcpCenter->registerCommands(CMD_100, __AppLogin);			// 转发给中心服务器的CMD_100
		__TcpCenter->registerCommands(CMD_200, __AppSelectRole);	// 选择角色，正式进入游戏
		__TcpCenter->registerCommands(CMD_300, __AppSelectRole);	// 创建角色
		__TcpCenter->registerCommands(CMD_400, __AppSelectRole);	// 删除角色

		// 这应该聊天服务器
		if(__TcpCenter)
			__TcpCenter->registerCommands(CMD_2000, __AppChat);

		len = __TcpGame.size();
		for (int i = 0; i < len; i++)
		{
			__TcpGame[i]->registerCommands(CMD_40, __AppGate);	// 收到游戏服务器发送来的负载信息

			__TcpGame[i]->registerCommands(CMD_500, __AppSelectRole);
			__TcpGame[i]->registerCommands(CMD_600, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_602, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_700, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_710, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_720, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_730, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_740, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_750, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_760, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_770, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_900, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_901, __AppEntry);
			__TcpGame[i]->registerCommands(CMD_902, __AppEntry);

			//背包操作
			__TcpGame[i]->registerCommands(CMD_800, __AppBag);
			__TcpGame[i]->registerCommands(CMD_810, __AppBag);
			__TcpGame[i]->registerCommands(CMD_820, __AppBag);
			__TcpGame[i]->registerCommands(CMD_830, __AppBag);
			__TcpGame[i]->registerCommands(CMD_840, __AppBag);
			__TcpGame[i]->registerCommands(CMD_850, __AppBag);
			__TcpGame[i]->registerCommands(CMD_860, __AppBag);
			__TcpGame[i]->registerCommands(CMD_870, __AppBag);

			__TcpGame[i]->registerCommands(CMD_1000, __AppMove);
			__TcpGame[i]->registerCommands(CMD_1100, __AppMove);

			__TcpGame[i]->registerCommands(CMD_3000, __AppSkill);
			__TcpGame[i]->registerCommands(CMD_3100, __AppSkill);
			__TcpGame[i]->registerCommands(CMD_3200, __AppSkill);
			__TcpGame[i]->registerCommands(CMD_4000, __AppSkill);
			__TcpGame[i]->registerCommands(CMD_4100, __AppSkill);
			__TcpGame[i]->registerCommands(CMD_4200, __AppSkill);
			__TcpGame[i]->registerCommands(CMD_5000, __AppSkill);

			__TcpGame[i]->registerCommands(CMD_6000, __AppDrop);
			__TcpGame[i]->registerCommands(CMD_6100, __AppDrop);
			__TcpGame[i]->registerCommands(CMD_6200, __AppDrop);

			__TcpGame[i]->registerCommands(CMD_8000, __AppRobot);
			__TcpGame[i]->registerCommands(CMD_8100, __AppRobot);
			__TcpGame[i]->registerCommands(CMD_8200, __AppRobot);
			__TcpGame[i]->registerCommands(CMD_8300, __AppRobot);
			__TcpGame[i]->registerCommands(CMD_8400, __AppRobot);
			__TcpGame[i]->registerCommands(CMD_8500, __AppRobot);
			__TcpGame[i]->registerCommands(CMD_8700, __AppRobot);
			__TcpGame[i]->registerCommands(CMD_8800, __AppRobot);
			// __TcpGame[i]->registerCommands(CMD_1200, __AppMove);
		}

		while (true)
		{
			onUpdate();
#ifdef  ___WIN32_
			Sleep(5);
#else
			usleep(5000);
#endif
		}
	}

	int run()
	{
		if (__AppManager == nullptr)
		{
			__AppManager = new AppManager();
			__AppManager->init();
		}

		return 0;
	}
}

