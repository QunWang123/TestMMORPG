// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Engine/World.h"
#include "AppManager.h"
#include "CommandData.h"
#include "RobotData.h"
#include "RobotBase.h"
#include <vector>
#include "MyCharacterBase.h"
#include "UDPClient.h"

UMyGameInstance* __AppGameInstance = nullptr;

int UMyGameInstance::AppInitGameInstance()
{
	auto www = GetWorld();
	__AppGameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	// time = __AppGameInstance->GetTimeSeconds();
	app::run();
	return 0;	
}

int32 UMyGameInstance::GetTimeSeconds()
{
	int32 ftime = GetWorld()->GetTimeSeconds() * 1000;
	return ftime;
}

void UMyGameInstance::Shutdown()
{
#ifdef TEST_UDP
	if (__udp != nullptr)
	{
		if (__udp->getData()->state == S_UConnect)
		{
			//通知服务器
			net::S_DATA_HEAD  d;
			d.ID = __udp->getData()->ID;
			d.cmd = CMD_65003;
			d.setSecure(func::__UDPClientInfo->RCode);
			__udp->sendData(&d, sizeof(net::S_DATA_HEAD), net::SPT_UDP);
		}
		__udp->getData()->reset();
	}
#endif

	for (auto it = __robotDatas.CreateConstIterator(); it; ++it)
	{
		FROBOT* robot = (FROBOT*)& it.Value();
		if (robot->view == NULL) continue;
		ARobotBase * view = (ARobotBase*)robot->view;
		if (view != NULL) view->Destroy();
	}
	clearAllRobot();

	if (app::__TcpClient != nullptr)
	{
		app::__TcpClient->setThread(true);
		app::__TcpClient->resetIP(app::login_ip, app::login_port);
		app::__TcpClient->disconnectServer(8000, "Shutdown");
	}
	Super::Shutdown();
}
