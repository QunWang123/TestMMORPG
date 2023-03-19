#include "AppMove.h"
#include "UserData.h"
#include "AppManager.h"
#include "MyCharacterBase.h"
#include "MyGameInstance.h"

#pragma optimize("", off)
namespace app
{
	AppMove* __AppMove = nullptr;

	AppMove::AppMove()
	{

	}

	AppMove::~AppMove()
	{

	}

	void AppMove::onInit()
	{

	}

	// 因为正常情况下不可能收到1000指令，收到了必定是出错了
	void onCMD_1000(net::TCPClient* tc)
	{
		uint16 childcmd = 0;
		tc->read(childcmd);
		FVector pos;
		if (childcmd == 3000)
		{
			FS_VECTOR _pos;
			tc->read(&_pos, sizeof(FS_VECTOR));
			pos.X = _pos.x;
			pos.Y = _pos.y;
			pos.Z = _pos.z;

			FString str = FString::Printf(TEXT("%d,%d,%d,%d"),
				(int32)E_EVENT_TYPE::EVENT_TOSRC,
				_pos.x, _pos.y, _pos.z);
			__myUserData.events.event_List.Add(str);
			// if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Black, str);
			return;
		}
		
		// if (__MyCharacter != NULL) __MyCharacter->onBackPosCommand(childcmd, pos);
	}

	void onCMD_1100(net::TCPClient* tc)
	{
		int32 userindex;
		int16 face;
		int32 speed;
		FS_VECTOR _pos;
		
		tc->read(userindex);
		tc->read(face);
		tc->read(speed);
		tc->read(&_pos, sizeof(FS_VECTOR));

		FVector targetpos;
		targetpos.X = _pos.x;
		targetpos.Y = _pos.y;
		targetpos.Z = _pos.z;

		FOTHER_ROLE_BASE* d = onLineDatas.Find(userindex);
		if (d != NULL) d->pos = targetpos;


		if (__AppGameInstance != NULL) __AppGameInstance->onMoveCommand(userindex, face, speed, targetpos);
	}

	bool AppMove::onClientCommand(net::TCPClient* tc, const uint16 cmd)
	{
		switch (cmd)
		{
		case CMD_1000:
			onCMD_1000(tc);
			break;
		case CMD_1100:
			onCMD_1100(tc);
			break;
		}


		return true;
	}

	void AppMove::send_Move(float _face, float _speed, FVector _pos, FVector _targetpos)
	{
		int16 face = _face * 100;
		int32 speed = _speed * 100;

		FS_VECTOR pos;
		FS_VECTOR targetpos;

		pos.x = _pos.X;
		pos.y = _pos.Y;
		pos.z = _pos.Z;

		targetpos.x = _targetpos.X;
		targetpos.y = _targetpos.Y;
		targetpos.z = _targetpos.Z;

		__TcpClient->begin(CMD_1000);
		__TcpClient->sss(face);
		__TcpClient->sss(speed);
		__TcpClient->sss(&pos, sizeof(FS_VECTOR));
		__TcpClient->sss(&targetpos, sizeof(FS_VECTOR));
		__TcpClient->end();
	}

}



#pragma optimize("", on)