// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintFunction.h"
#include "AppManager.h"
#include "Engine/Engine.h"
#include "MyGameInstance.h"
#include "AppSelectRole.h"
#include "AppEntryWorld.h"
#include "AppMove.h"
#include "AppBag.h"
#include "AppChat.h"
#include "WorldScript.h"

int32 UBlueprintFunction::AppUpdate()
{
	app::onUpdate();
	return 0;
}

void UBlueprintFunction::DisConnect()
{
	if (app::__TcpClient == nullptr) return;
	app::__TcpClient->disconnectServer(6000, "blue close");
}

bool UBlueprintFunction::isSecurity()
{
	if (app::__TcpClient == NULL) return false;
	uint8 state = app::__TcpClient->getData()->state;
	if (state < func::C_CONNECT_SECURE) return false;
	return true;
}

bool UBlueprintFunction::isLogin()
{
	if (app::__TcpClient == NULL) return false;
	if (app::__TcpClient->getData()->state == func::C_LOGIN) return true;

	return false;
}

void UBlueprintFunction::InitHeartTime()
{
	if (__AppGameInstance == NULL ||
		app::__TcpClient == NULL) return;

	app::__TcpClient->getData()->time_Heart = __AppGameInstance->GetTimeSeconds();
}

int32 UBlueprintFunction::GetDelayTime()
{
	return delaytime;
}

int32 UBlueprintFunction::read_int8()
{
	int8 temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

int32 UBlueprintFunction::read_int16()
{
	uint16 temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

int32 UBlueprintFunction::read_int32()
{
	int32 temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

float UBlueprintFunction::read_float()
{
	float temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

bool UBlueprintFunction::read_bool()
{
	bool temp = 0;
	app::__TcpClient->read(temp);
	return temp;
}

FVector UBlueprintFunction::read_FVector()
{
	FVector temp;
	app::__TcpClient->read(&temp, sizeof(FVector));
	return temp;
}

FRotator UBlueprintFunction::read_FRotator()
{
	FRotator temp;
	app::__TcpClient->read(&temp, sizeof(FRotator));
	return temp;
}

FString UBlueprintFunction::read_FString()
{
	int32 len = 0;
	app::__TcpClient->read(len);
	uint8* cc = (uint8*)FMemory::Malloc(len);
	app::__TcpClient->read(cc, len);
	FString temp = UTF8_TO_TCHAR(cc);
	FMemory::Free(cc);
	return temp;
}

FString UBlueprintFunction::read_FString_len(int32 len)
{
	uint8* cc = (uint8*)FMemory::Malloc(len);
	app::__TcpClient->read(cc, len);
	FString temp = UTF8_TO_TCHAR(cc);
	FMemory::Free(cc);
	return temp;
}

FPlayerBase UBlueprintFunction::read_FPlayerBase()
{
	FPlayerBase data;
	app::__TcpClient->read(&data, 48);

	data.nick = read_FString_len(20);
	return data;
}

FROLE_PROP UBlueprintFunction::read_FROLE_PROP()
{
	FROLE_PROP prop;
	uint16	tempvalue = 0;
	// FROLE_PROP_BASE
	app::__TcpClient->read(&prop, 6);
	app::__TcpClient->read(tempvalue);
	prop.base.count = (int32)tempvalue;
	app::__TcpClient->read(prop.base.createtime);
	app::__TcpClient->read(prop.base.uniqueid);
	app::__TcpClient->read(prop.base.money);
	// FROLE_PROP_EQUIP
	app::__TcpClient->read(tempvalue);
	prop.equip.p_atk = (int32)tempvalue;
	app::__TcpClient->read(tempvalue);
	prop.equip.m_atk = (int32)tempvalue;
	app::__TcpClient->read(tempvalue);
	prop.equip.p_defend = (int32)tempvalue;
	app::__TcpClient->read(tempvalue);
	prop.equip.m_defend = (int32)tempvalue;
	app::__TcpClient->read(prop.equip.crit);
	app::__TcpClient->read(prop.equip.dodge);
	app::__TcpClient->read(tempvalue);
	prop.equip.hp = (int32)tempvalue;
	app::__TcpClient->read(tempvalue);
	prop.equip.mp = (int32)tempvalue;
	prop.equip.nick = UBlueprintFunction::read_FString_len(USER_MAX_NICK);
	// FROLE_PROP_GEM
	app::__TcpClient->read(&prop.gem, sizeof(FROLE_PROP_GEM));
	return prop;
}

// 虽然cmd看起来是4个字节，但其实不能超过两个字节，最大命令号是65535
void UBlueprintFunction::send_begin(int32 cmd)
{
	app::__TcpClient->begin(cmd);
}

void UBlueprintFunction::send_end()
{
	app::__TcpClient->end();
}

void UBlueprintFunction::send_int8(int32 value)
{
	app::__TcpClient->sss((int8)value);
}

void UBlueprintFunction::send_int16(int32 value)
{
	app::__TcpClient->sss((int16)value);
}

void UBlueprintFunction::send_int32(int32 value)
{
	app::__TcpClient->sss(value);
}

void UBlueprintFunction::send_float(float value)
{
	app::__TcpClient->sss(value);
}

void UBlueprintFunction::send_bool(bool value)
{
	app::__TcpClient->sss(value);
}

void UBlueprintFunction::send_FVector(FVector value)
{
	app::__TcpClient->sss(&value, sizeof(FVector));
}

void UBlueprintFunction::send_FRotator(FRotator value)
{
	app::__TcpClient->sss(&value, sizeof(FRotator));
}

void UBlueprintFunction::send_FString(FString value)
{
	TCHAR* pdata = value.GetCharArray().GetData();
	uint8* cc = (uint8*)TCHAR_TO_UTF8(pdata);

	int32 size = value.GetCharArray().Num();
	app::__TcpClient->sss(size);
	app::__TcpClient->sss(cc, size);
}

char senddata[10240];
// 一次就是发len个长度，无论原数据有多长
void UBlueprintFunction::send_FString_len(FString value, int32 len)
{
	if (value.Len() < 1) return;

	FMemory::Memset(&senddata[0], 0, 10240);
	TCHAR* pdata = value.GetCharArray().GetData();
	uint8* cc = (uint8*)TCHAR_TO_UTF8(pdata);
	
	int size = value.GetCharArray().Num();
	if (size > len) size = len;

	FMemory::Memcpy(senddata, cc, len);
	app::__TcpClient->sss(&senddata[0], len);
}

ACharacterBase* UBlueprintFunction::findUserView(int32 userindex)
{
	FOTHER_ROLE_BASE* d = onLineDatas.Find(userindex);
	if (d == nullptr || d->view == nullptr) return nullptr;
	ACharacterBase* view = (ACharacterBase*)d->view;
	return view;
}

FVector UBlueprintFunction::GetMapLeftPos()
{
	return __CurMapInfo.leftpos;
}

FString UBlueprintFunction::GetJob(int32 JonIndex)
{
	FString ss;
	switch (JonIndex)
	{
	case 1:
		return FString("job1");
	case 2:
		return FString("job2");
	case 3:
		return FString("job3");
	}
	return FString("unkonwn");
}


void UBlueprintFunction::send_ChatText(FString text)
{
	app::__AppChat->send_ChatText(text);
}

void UBlueprintFunction::send_GetBag()
{
	app::AppBag::send_GetBag();
}

void UBlueprintFunction::send_GetCombat(int32 userindex)
{
	app::AppBag::send_GetCombat(userindex);
}

void UBlueprintFunction::send_CombaineBag()
{
	app::AppBag::send_CombaineBag();
}

void UBlueprintFunction::send_ChangeMap(int32 mapid)
{
	app::AppEntryWorld::send_ChangeMap(mapid);
}

void UBlueprintFunction::send_ChangeLine(uint8 line)
{
	app::AppEntryWorld::send_ChangeLine(line);
}

FString UBlueprintFunction::GetMapName(int32 mapid)
{
	script::SCRIPT_MAP_BASE* mapscript = script::findScript_Map(mapid);
	if (!mapscript) return FString::FString("");
	return mapscript->mapname;
}
