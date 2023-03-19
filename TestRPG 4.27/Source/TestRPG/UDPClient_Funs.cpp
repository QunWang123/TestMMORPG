#include "UDPClient.h"
#include "MyGameInstance.h"
#include "IDefine.h"

using namespace func;
using namespace net;


void UDPClient::parseCommand()
{
	if (data.state == S_UFree)
	{
		connectServer(false);
		return;
	}
	//checkConnect();
	if (data.state == S_UFree) return;

// 	S_DATA_HEAD  Tmpd;
// 	Tmpd.ID = data.ID;
// 	Tmpd.cmd = 3000;
// 	Tmpd.setSecure(func::__UDPClientInfo->RCode);
// 	sendData(&Tmpd, sizeof(S_DATA_HEAD), net::SPT_KCP);

	updateKcp();
	sendHeart();

	auto c = getData();
	while (c->recv_Tail - c->recv_Head >= 10)
	{
		//1、解析头
		char head[2];
		head[0] = c->recvBuf[c->recv_Head];
		head[1] = c->recvBuf[c->recv_Head + 1];

		if (head[0] != func::__UDPClientInfo->Head[0] || head[1] != func::__UDPClientInfo->Head[1])
		{
			return;
		}
		uint16 length = (*(uint16*)(c->recvBuf + c->recv_Head + 2));

		S_DATA_HEAD d;
		memcpy(&d, &c->recvBuf[c->recv_Head + 4], sizeof(S_DATA_HEAD));
		d.getSecure(func::__UDPClientInfo->RCode);

		//2、长度不够 需要继续等待 
		if (c->recv_Tail < c->recv_Head + length)
		{
			return;
		}
		c->recv_TempHead = c->recv_Head + 4;
		c->recv_TempTail = c->recv_Head + length;
		parseCommand(d.cmd);
		//4、增加读取长度
		c->recv_Head += length;
	}
}
void UDPClient::parseCommand(uint16 cmd)
{
	this->data.time_HeartConnect = __AppGameInstance->GetTimeSeconds();
	switch (cmd)
	{
	case CMD_65002:
		{
			FString ss = FString::Printf(TEXT("recv heart"));
			if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 35, FColor::Emerald, ss);
		}
		break;
	}

	if (onCommandtEvent != nullptr) this->onCommandtEvent(this, cmd, TEXT(""));
}



void UDPClient::setOnUdpServerAccept(UDPCLIENTNOTIFY_EVENT event)
{
	onAcceptEvent = event;
}

void UDPClient::setOnUdpServerDisconnect(UDPCLIENTNOTIFY_EVENT event)
{
	onDisconnectEvent = event;
}

void UDPClient::setOnUdpServerCommand(UDPCLIENTNOTIFY_EVENT event)
{
	onCommandtEvent = event;
}


void UDPClient::checkConnect()
{
	int tempTime = __AppGameInstance->GetTimeSeconds() - data.time_HeartConnect;
	if (tempTime < func::__UDPClientInfo->HeartTimeMax) return;
	

	if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, 1001, "heart timeout");
}


void UDPClient::connectServer(bool isfirst)
{
	if (data.state == S_UConnect) return;

	if (isfirst == false)
	{
		int tempTime = __AppGameInstance->GetTimeSeconds() - data.time_AutoConnect;
		if (tempTime < func::__UDPClientInfo->AutoTime) return;
		data.time_AutoConnect = __AppGameInstance->GetTimeSeconds();
	}


	S_DATA_HEAD d;
	d.ID = func::__UDPClientInfo->ID;
	d.cmd = CMD_65001;
	d.setSecure(func::__UDPClientInfo->RCode);


	sendData(&d, sizeof(S_DATA_HEAD),net::SPT_UDP);

	FString ss = FString::Printf(TEXT("udp connectServer "));
	// if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 5, FColor::Emerald, ss);
}

void UDPClient::sendHeart()
{
	if (data.state == S_UFree) return;

	int temp = __AppGameInstance->GetTimeSeconds() - data.time_Heart;
	if (temp < func::__UDPClientInfo->HeartTime) return;

	data.time_Heart = __AppGameInstance->GetTimeSeconds();
	//返回连接ID
	S_DATA_HEAD  d;
	d.ID = data.ID;
	d.cmd = CMD_65002;
	d.setSecure(func::__UDPClientInfo->RCode);
	sendData(&d, sizeof(S_DATA_HEAD), net::SPT_UDP);

	FString ss = FString::Printf(TEXT("sendHeart"));
	// if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 5, FColor::Emerald, ss);
}
void UDPClient::readData(void* v, const int32 len)
{
	auto c = getData();
	if (c == nullptr) return;

	if (c->ID == -1 ||
		c->recv_TempTail == 0 ||
		c->recvBuf == nullptr ||
		c->recv_TempHead + len > c->recv_TempTail)
	{
		return;
	}

	memcpy(v, &c->recvBuf[c->recv_TempHead], len);
	c->recv_TempHead += len;
}

void UDPClient::clearConnect()
{
	releaseKcp();
	this->data.reset();
}

