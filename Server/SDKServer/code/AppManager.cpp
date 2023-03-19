#include "AppManager.h"
#include "AppGlobal.h"
#include "ShareFunction.h"
#include "UserManager.h"
#include "DBManager.h"
#include "CommandData.h"


#ifndef  ___WIN32_
#include <unistd.h>
#endif


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
		temp_time = time(NULL);

		int concount = 0;
		int securtiycount = 0;
		__TcpServer->getSecurityCount(concount, securtiycount);

		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);
		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);

		sprintf_s(printfstr, "[SDK:%d-%d] connect-%d  security-%d [memory:%.4f/%.4f]",
			func::__ServerInfo->ID, func::__ServerInfo->Port, concount, securtiycount, curmemory1, maxmemory1);
		SetWindowTextA(GetConsoleWindow(), printfstr);
#endif
	}
	void onUpdate()
	{
		if (__TcpServer == nullptr) return;

		__TcpServer->parseCommand();

		printInfo();

	}

	//创建HTTP服务器
	void runHttpServer()
	{
		__HttpServer = new http::HttpServer();
		__HttpServer->runServer();
	}
	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		if (func::__ServerListInfo.size() < 1) return;

		// io::runLogThread();

		//创建http服务器
		runHttpServer();

		//创建服务器 启动
		__TcpServer = net::NewTcpServer();
		__TcpServer->setOnClientAccept(onClientAccept);
		__TcpServer->setOnClientSecureConnect(onClientSecureConnect);
		__TcpServer->setOnClientDisConnect(onClientDisConnect);
		__TcpServer->setOnClientTimeOut(onClientTimeout);
		__TcpServer->setOnClientExcept(onClientExcept);
		__TcpServer->runServer(1);


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

