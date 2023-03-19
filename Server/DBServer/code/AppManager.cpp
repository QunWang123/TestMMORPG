#include "AppManager.h"
#include "AppGlobal.h"
#include "../../share/ShareFunction.h"

#include "DBManager.h"
#ifndef  ___WIN32_
#include <unistd.h>
#endif
#include "AppLogin.h"
#include "UserManager.h"
#include "AppSelectRole.h"
#include "AppGate.h"
#include "AppGame.h"

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

		int count = __UserManager->__MemberStates.size();

		sprintf_s(printfstr, "DB [%d-%d] connect-%d, secureconnect-%d, onlines:%d", 
			func::__ServerInfo->ID, func::__ServerInfo->Port, connectcount, secureconnectcount, count);
		SetWindowTextA(GetConsoleWindow(), printfstr);
#endif
	}
	void onUpdate()
	{
		if (__TcpServer == nullptr) return;

		__TcpServer->parseCommand();
		__UserManager->updata();
		__DBManager->update();
		printInfo();

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

		// __TcpServer->registerCommand(CMD_REIGSTER, __AppPlayer);
		
		__UserManager = new UserManager();
		__DBManager = new DBManager();
		__DBManager->InitDB();
		__AppLogin = new AppLogin();
		__AppSelectRole = new AppSelectRole();
		__AppGate = new AppGate();
		__AppGame = new AppGame();

		__TcpServer->registerCommands(CMD_40, __AppGate);
		__TcpServer->registerCommands(CMD_90, __AppLogin);
		__TcpServer->registerCommands(CMD_100, __AppLogin);
		__TcpServer->registerCommands(CMD_600, __AppLogin);		// 离线，保存基础数据
		__TcpServer->registerCommands(CMD_610, __AppLogin);

		__TcpServer->registerCommands(CMD_200, __AppSelectRole);
		__TcpServer->registerCommands(CMD_300, __AppSelectRole);
		__TcpServer->registerCommands(CMD_400, __AppSelectRole);
		__TcpServer->registerCommands(CMD_500, __AppSelectRole);		// 加载角色数据
		__TcpServer->registerCommands(CMD_901, __AppSelectRole);		// 切换地图加载角色数据
		__TcpServer->registerCommands(CMD_902, __AppSelectRole);		// 切换线路加载角色数据
		__TcpServer->registerCommands(CMD_7200, __AppSelectRole);//打开副本加载角色数据
		__TcpServer->registerCommands(CMD_7300, __AppSelectRole);//打开副本加载角色数据
		__TcpServer->registerCommands(CMD_7400, __AppSelectRole);
		__TcpServer->registerCommands(CMD_7301, __AppSelectRole);//组队队友进入副本

		__TcpServer->registerCommands(CMD_731, __AppGame);
		__TcpServer->registerCommands(CMD_740, __AppGame);
		__TcpServer->registerCommands(CMD_750, __AppGame);
		__TcpServer->registerCommands(CMD_760, __AppGame);
		__TcpServer->registerCommands(CMD_800, __AppGame);//背包更新数据
		__TcpServer->registerCommands(CMD_810, __AppGame);
		
		while (true)
		{
			onUpdate();
#ifdef  ___WIN32_
			Sleep(5);
#else
			usleep(2);
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

