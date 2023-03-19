#include "AppManager.h"
#include "AppGlobal.h"
#include "../../share/ShareFunction.h"

#include "DBManager.h"
#ifndef  ___WIN32_
#include <unistd.h>
#endif
#include "UserManager.h"
#include "CommandData.h"
#include "AppLogin.h"
#include "AppGate.h"
#include "TestJson.h"

#define TESTCONNECT 1

namespace app
{
	AppManager* __AppManager = nullptr;
	int temp_time_app = 0;
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
		int tempTime = (int)time(NULL) - temp_time_app;
		if (tempTime < 1) return;

		int connectcount = 0;
		int secureconnectcount = 0;
		__TcpServer->getSecurityCount(connectcount, secureconnectcount);


		sprintf_s(printfstr, "LoginServer [%d-%d] connect-%d, secureconnect-%d", func::__ServerInfo->ID, func::__ServerInfo->Port, connectcount, secureconnectcount);
		SetWindowTextA(GetConsoleWindow(), printfstr);
#endif
	}
	void onUpdate()
	{
		if (__TcpServer == nullptr) return;

		__TcpServer->parseCommand();
		__TcpCenter->parseCommand();
		// __DBManager->update();
		__AppGate->onUpdate();
		json::updateJson();
		printInfo();

	}
	void runHttpClient()
	{
		http::initPath();
		// MAX_HTTP_THREAD 3
		for (int i = 0; i < MAX_HTTP_THREAD; i++)
		{
			__HttpClient[i] = new http::HttpClient();
			__HttpClient[i]->id = i;
		}
	}
	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		if (func::__ServerListInfo.size() < 1) return;

		runHttpClient();

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
		for (int i = 0; i < len; i++)
		{
			auto xml = func::__ServerListInfo[i];
			u32 type = share::serverIDToType(xml->ID);
			if (type != func::S_TYPE_CENTER) continue;

			__TcpCenter = net::NewTcpClient();
			__TcpCenter->setOnConnect(onConnect);
			__TcpCenter->setOnDisConnect(onDisConnect);
			__TcpCenter->setOnExcept(onExceptConnect);
			__TcpCenter->setOnSecureConnect(onSecureConnect);
			__TcpCenter->runClient(xml->ID, xml->IP, xml->Port);
			__TcpCenter->getData()->ID = i;
			break;
		}
		

		__UserManager = new UserManager();
// 		__DBManager = new DBManager();
// 		__DBManager->InitDB();
		__AppLogin = new AppLogin();
		__AppGate = new AppGate();
		
		__TcpServer->registerCommands(CMD_HEART, __AppLogin);
		__TcpServer->registerCommands(CMD_10, __AppLogin);
		__TcpServer->registerCommands(CMD_20, __AppLogin);

		__TcpCenter->registerCommands(CMD_40, __AppGate);

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

