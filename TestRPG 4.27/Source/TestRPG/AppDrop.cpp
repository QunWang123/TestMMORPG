
#include  "AppDrop.h"
#include "AppManager.h"
#include "CommandData.h"
#include "UserData.h"
#include "MyCharacterBase.h"
#include "MyGameInstance.h"
#include "WorldTools.h"
namespace app
{
	IContainer*  __AppDrop;
	TMap<int32, int32> __SendTimes_Drop;//����DROP����ʱ��

	AppDrop::AppDrop()
	{
	}
	AppDrop::~AppDrop()
	{
	}
	void AppDrop::onInit()
	{
	}
	//�Ƿ��ܷ����������� ��Ҫ���ڿ���
	bool isSendDropData(int32 dropindex)
	{
		auto it = __SendTimes_Drop.Find(dropindex);
		if (it != nullptr)
		{
			int32 ftime = *it;
			ftime = __AppGameInstance->GetTimeSeconds() - ftime;
			if (ftime < 2000 && ftime > 0)
			{
				FString ss = FString::Printf(TEXT("getDropdata is faster...%d"), ftime);
				if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 5, FColor::Emerald, ss);
				return false;
			}

			__SendTimes_Drop.Remove(dropindex);
		}

		__SendTimes_Drop.Add(dropindex, __AppGameInstance->GetTimeSeconds());
		return true;
	}
	
	void onCMD_6000(net::TCPClient* tc)
	{
		FDROP_BASE drop;
		int16 err;
		tc->read(err);
		drop.dropindex = err;
		//����ת��
		gridToPos(&drop.grid, &drop.pos, &__CurMapInfo.leftpos);

		if (__AppGameInstance != nullptr) __AppGameInstance->onDropCommand(CMD_6000, drop);

	}
	void onCMD_6100(net::TCPClient* tc)
	{
		FDROP_BASE drop;
		tc->read(&drop,28);
		
		//����ת��
		gridToPos(&drop.grid, &drop.pos, &__CurMapInfo.leftpos);

		if (__AppGameInstance != nullptr) __AppGameInstance->onDropCommand(CMD_6100, drop);
		
	}
	void onCMD_6200(net::TCPClient* tc)
	{
		FDROP_BASE drop;
		tc->read(drop.dropindex);

		if (__AppGameInstance != nullptr) __AppGameInstance->onDropCommand(CMD_6200, drop);
	}
	//�����յ����ƶ�����
	bool AppDrop::onClientCommand(net::TCPClient* tc, const uint16 cmd)
	{
		switch (cmd)
		{
		case CMD_6000: //���ش���
			onCMD_6000(tc);
			break;
		case CMD_6100: //��������
			onCMD_6100(tc);
			break;
		case CMD_6200://ɾ������
			onCMD_6200(tc);
			break;
		}
		return true;
	}

	void AppDrop::send_GetDrop(int32 dropindex, int32 dropid)
	{
		if (isSendDropData(dropindex) == false) return;

		app::__TcpClient->begin(CMD_6000);
		app::__TcpClient->sss(dropindex);
		app::__TcpClient->sss(dropid);
		app::__TcpClient->end();
	}
}
