#include "AppManager.h"
#include "AppGlobal.h"
#include "../../share/ShareFunction.h"

// #include "DBManager.h"
#ifndef  ___WIN32_
#include <unistd.h>
#endif
#include "AppSelectRole.h"
#include "UserManager.h"
#include "AppEnterWorld.h"
#include "WorldMap.h"
#include "AppMove.h"
#include "WorldScript.h"
#include "AppSkill.h"
#include <thread>
#include <chrono>
#include "RobotManager.h"
#include "WorldTools.h"
#include "AppRobot.h"
#include "AppGate.h"
#include "AppBag.h"
#include "AppDrop.h"
#include "AppCopy.h"
#include "AppTeam.h"

#define TESTCONNECT 1

namespace app
{
	AppManager* __AppManager = nullptr;
	int temp_time = 0;
	char printfstr[1000];
	std::chrono::steady_clock::time_point temp_Checktime;

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
		if (tempTime < 2) return;

		int concount = 0;
		int securtiycount = 0;
		__TcpServer->getSecurityCount(concount, securtiycount);

		int count = __UserManager->__OnlineCount;
		int robot_count = __RobotManager->count;
		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);
		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);

		int room_Count = 0;
// 		for (u32 i = 0; i < MAX_LAYER_COUNT; i++)
// 		{
// 			S_COPY_ROOM* room = __AppCopy->findRoom(i);
// 			if (room == nullptr) continue;
// 			if (room->state > 0) continue;
// 			room_Count++;
// 		}

		sprintf_s(printfstr, "[Game-%d:%d line:%d] [con:%d/%d] online:%d  robot:%d [m:%.2f/%.2f] copy:%d",
			func::__ServerInfo->Port, func::__ServerInfo->ID, share::__ServerLine,
			concount, securtiycount, count, robot_count, curmemory1, maxmemory1, room_Count);
		SetWindowTextA(GetConsoleWindow(), printfstr);
#endif
	}
	void onUpdate()
	{
		if (__TcpServer == nullptr) return;

		__UserManager->update();
		__RobotManager->update();
		__TcpDB->parseCommand();
		__TcpCenter->parseCommand();
		__TcpServer->parseCommand();
		__AppDrop->onUpdate();
		__AppCopy->onUpdate();
		printInfo();
	}

	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		// 初始化脚本
		script::initScript();
		// 初始化地图
		__World->LoadMap();
		TS_Tools::calculateRectanglePoints();
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
		
		int len = func::__ServerListInfo.size();
		for (int i = 0; i < len; i++)
		{
			auto xml = func::__ServerListInfo[i];
			auto client = net::NewTcpClient();
			client->setOnConnect(onConnect);
			client->setOnDisConnect(onDisConnect);
			client->setOnExcept(onExceptConnect);
			client->setOnSecureConnect(onSecureConnect);

			if (xml->ID >= 1000 && xml->ID < 2000)
			{
				__TcpDB = client;
			}
			else if (xml->ID >= 2000 && xml->ID < 3000)
			{
				__TcpCenter = client;
			}

			client->runClient(xml->ID, xml->IP, xml->Port);
			client->getData()->ID = 0;
		}

		__UserManager = new UserManager();
		__AppSelectRole = new AppSelectRole();
		__AppEntry = new AppEnterWorld();
		__AppMove = new AppMove();
		__AppSkill = new AppSkill();
		__RobotManager = new RobotManager();
		__AppRobot = new AppRobot();
		__AppGate = new AppGate();
		__AppDrop = new AppDrop();
		__AppBag = new AppBag();
		__AppTeam = new AppTeam();
		__AppCopy = new AppCopy();

		__TcpDB->registerCommands(CMD_500, __AppSelectRole);
		__TcpDB->registerCommands(CMD_901, __AppEntry);
		__TcpDB->registerCommands(CMD_902, __AppEntry);

		__TcpDB->registerCommands(CMD_7200, __AppCopy);
		__TcpDB->registerCommands(CMD_7300, __AppCopy);
		__TcpDB->registerCommands(CMD_7400, __AppCopy);
		__TcpDB->registerCommands(CMD_7301, __AppCopy);

		__TcpServer->registerCommands(CMD_40, __AppGate);
		__TcpServer->registerCommands(CMD_500, __AppSelectRole);
		__TcpServer->registerCommands(CMD_600, __AppSelectRole);	// 玩家掉线
		__TcpServer->registerCommands(CMD_90, __AppSelectRole);		// 网关服务器发送来的心跳包
		__TcpServer->registerCommands(CMD_700, __AppEntry);
		__TcpServer->registerCommands(CMD_900, __AppEntry);
		__TcpServer->registerCommands(CMD_901, __AppEntry);
		__TcpServer->registerCommands(CMD_902, __AppEntry);

		__TcpServer->registerCommands(CMD_820, __AppBag);
		__TcpServer->registerCommands(CMD_830, __AppBag);
		__TcpServer->registerCommands(CMD_840, __AppBag);
		__TcpServer->registerCommands(CMD_850, __AppBag);
		__TcpServer->registerCommands(CMD_860, __AppBag);
		__TcpServer->registerCommands(CMD_870, __AppBag);
		__TcpServer->registerCommands(CMD_890, __AppBag);

		__TcpServer->registerCommands(CMD_1000, __AppMove);
		__TcpServer->registerCommands(CMD_3000, __AppSkill);
		__TcpServer->registerCommands(CMD_4100, __AppSkill);

		__TcpServer->registerCommands(CMD_6000, __AppDrop);
		__TcpServer->registerCommands(CMD_6100, __AppDrop);
		__TcpServer->registerCommands(CMD_6200, __AppDrop);

		//副本操作
		__TcpServer->registerCommands(CMD_7200, __AppCopy);
		__TcpServer->registerCommands(CMD_7300, __AppCopy);
		__TcpServer->registerCommands(CMD_7400, __AppCopy);
		__TcpServer->registerCommands(CMD_7301, __AppCopy);
		
		__TcpServer->registerCommands(CMD_8000, __AppRobot);

		//组队操作更新
		if (__TcpCenter != nullptr)
		{
			__TcpCenter->registerCommands(CMD_7040, __AppTeam);
			__TcpCenter->registerCommands(CMD_7500, __AppTeam);
			__TcpCenter->registerCommands(CMD_7600, __AppTeam);
			__TcpCenter->registerCommands(CMD_7301, __AppCopy);
		}

		temp_Checktime = std::chrono::steady_clock::now();

		while (true)
		{
			onUpdate();
			std::this_thread::sleep_for(std::chrono::milliseconds(5));

			auto current = std::chrono::steady_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - temp_Checktime);
			// global_gametime += duration.count();
			global_gametime = duration.count();
			// temp_Checktime = std::chrono::steady_clock::now();
// #ifdef  ___WIN32_
// 			Sleep(5);
// #else
// 			usleep(2);
// #endif
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

