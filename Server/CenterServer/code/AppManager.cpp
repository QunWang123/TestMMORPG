#include "AppManager.h"
#include "AppGlobal.h"
#include "../../share/ShareFunction.h"

// #include "DBManager.h"
#ifndef  ___WIN32_
#include <unistd.h>
#endif
#include "UserManager.h"
#include "AppLogin.h"
#include "AppSelectRole.h"
#include "AppGate.h"
#include "AppChat.h"
#include "AppEnterWorld.h"

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


	//��ӡ��Ϣ
	void printInfo()
	{
#ifdef  ___WIN32_
		int tempTime = (int)time(NULL) - temp_time_app;
		if (tempTime < 1) return;

		int connectcount = 0;
		int secureconnectcount = 0;
		__TcpServer->getSecurityCount(connectcount, secureconnectcount);

		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);
		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);

		s32 count = __UserManager->getRequestCount();
		s32 online = __UserManager->__OnlineCount;

		sprintf_s(printfstr, "[Center:%d-%d] connect-%d  security-%d [queue:%d online:%d] [memory:%.2f/%.2f]",
			func::__ServerInfo->ID, func::__ServerInfo->Port, connectcount, secureconnectcount, count, online, curmemory1, maxmemory1);

		SetWindowTextA(GetConsoleWindow(), printfstr);
#endif
	}
	void onUpdate()
	{
		if (__TcpServer == nullptr) return;

		__TcpServer->parseCommand();
		__TcpDB->parseCommand();
		__UserManager->update();
		__AppGate->onUpdate();
		printInfo();
	}

	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		if (func::__ServerListInfo.size() < 1) return;



		//���������� ����
		__TcpServer = net::NewTcpServer();
		__TcpServer->setOnClientAccept(onClientAccept);
		__TcpServer->setOnClientSecureConnect(onClientSecureConnect);
		__TcpServer->setOnClientDisConnect(onClientDisConnect);
		__TcpServer->setOnClientTimeOut(onClientTimeout);
		__TcpServer->setOnClientExcept(onClientExcept);
		__TcpServer->runServer(1);

		// __TcpServer->registerCommand(CMD_REIGSTER, __AppPlayer);
		
		auto xml = func::__ServerListInfo[0];
		__TcpDB = net::NewTcpClient();
		__TcpDB->setOnConnect(onConnect);
		__TcpDB->setOnDisConnect(onDisConnect);
		__TcpDB->setOnExcept(onExceptConnect);
		__TcpDB->setOnSecureConnect(onSecureConnect);

		__TcpDB->runClient(xml->ID, xml->IP, xml->Port);
		__TcpDB->getData()->ID = 0;
		

		//3 ���������������ڴ� ��������
		for (u32 i = 0; i < func::__ServerList.size(); i++)
		{
			auto xml = func::__ServerList[i];
			
			u32 type = share::serverIDToType(xml->ID);

			S_SERVER_MANAGER* s = new S_SERVER_MANAGER();
			s->reset();
			s->id = xml->ID;
			s->port = xml->Port;
			memcpy(s->ip, xml->IP, MAX_IP_LEN);

			if (type == func::S_TYPE_GATE)
			{
				__GateList.insert(std::make_pair(s->id, s));
				continue;
			}
			else if (type == func::S_TYPE_GAME)
			{
				__GameList.insert(std::make_pair(s->id, s));
				continue;
			}
			else if (type == func::S_TYPE_DB)
			{
				__DBList.insert(std::make_pair(s->id, s));
			}
		}

		__UserManager = new UserManager();
		__AppLogin = new AppLogin();
		__AppSelectRole = new AppSelectRole();
		__AppChat = new AppChat();
		__AppGate = new AppGate();
		__AppEnterWorld = new AppEnterWorld();
		
		__TcpServer->registerCommands(CMD_40, __AppGate);
		__TcpServer->registerCommands(CMD_10, __AppLogin);
		__TcpServer->registerCommands(CMD_90, __AppLogin);
		__TcpServer->registerCommands(CMD_100, __AppLogin);
		__TcpServer->registerCommands(CMD_600, __AppLogin);			// ����
		__TcpServer->registerCommands(CMD_900, __AppEnterWorld);			// ��ҽ��루�˴���ȫ���Զ���������Ϊһ���µ���ӷ�������
		__TcpServer->registerCommands(CMD_901, __AppEnterWorld);			// �л���ͼ���˴���ȫ���Զ���������Ϊһ���µ���ӷ�������
		__TcpServer->registerCommands(CMD_902, __AppEnterWorld);			// �л���·���˴���ȫ���Զ���������Ϊһ���µ���ӷ�������
		__TcpServer->registerCommands(CMD_200, __AppSelectRole);	// ѡ���ɫ����ʽ������Ϸ
		__TcpServer->registerCommands(CMD_300, __AppSelectRole);	// ������ɫ
		__TcpServer->registerCommands(CMD_400, __AppSelectRole);	// ɾ����ɫ

		__TcpServer->registerCommands(CMD_2000, __AppChat);			// ���죨�˴���ȫ���Զ���������Ϊһ���µ������������
		
		__TcpDB->registerCommands(CMD_40, __AppGate);
		__TcpDB->registerCommands(CMD_100, __AppLogin);
		__TcpDB->registerCommands(CMD_200, __AppSelectRole);		// ѡ���ɫ����ʽ������Ϸ
		__TcpDB->registerCommands(CMD_300, __AppSelectRole);		// ������ɫ
		__TcpDB->registerCommands(CMD_400, __AppSelectRole);		// ɾ����ɫ

// 		u32 curmemory = 0;
// 		u32 maxmemory = 0;
// 		share::updateMemory(curmemory, maxmemory);
// 		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
// 		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);
// 		io::pushLog(io::EFT_RUN, "runServer success:%d %d memory:%.4f/%.4f\n",
// 			func::__ServerInfo->Port, func::__ServerInfo->ID, curmemory1, maxmemory1);
// 		io::pushLog(io::EFT_ERR, "runServer success:%d %d memory:%.4f/%.4f\n",
// 			func::__ServerInfo->Port, func::__ServerInfo->ID, curmemory1, maxmemory1);

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

