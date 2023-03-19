// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "sockets.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/core/Public/HAL/Runnable.h"
#include "Runtime/core/Public/HAL/RunnableThread.h"
#include "IDefine_Udp.h"

using namespace net;

class TESTRPG_API UDPClient;
typedef void(*UDPCLIENTNOTIFY_EVENT)(UDPClient* udp, int32 code, FString err);

class UDPClient_Thread :public FRunnable
{
public:
	UDPClient_Thread();
	virtual ~UDPClient_Thread();
	virtual uint32 Run();
	virtual void Exit();
	void StopThread();
	FRunnableThread* thread;
};


/**
	*
	*/
class TESTRPG_API UDPClient
{
public:
	UDPClient();
	~UDPClient();
	void runClient(const FString& ip, int32 port);
	void resetIP(const FString& ip, int32 port);
	void closeClient();
	void run();
	void sendData(const void* buf,const int32 size, const FString& ip, int32 port);
	void sendData(const void* buf,const int32 size,const int8 protocolType);

	void  parseCommand();

	inline S_UDP_BASE* getData() { return &data; }
	 void  setOnUdpServerAccept(UDPCLIENTNOTIFY_EVENT event);
	 void  setOnUdpServerDisconnect(UDPCLIENTNOTIFY_EVENT event);
	 void  setOnUdpServerCommand(UDPCLIENTNOTIFY_EVENT event);

	 void readData(void* v, const int32 len);
	 void  clearConnect();
private:
	void runThread();
	void InitSocket();
	int recvData();
	void  checkConnect();
	
	void  connectServer(bool isfirst);
	void  sendHeart();
	void  parseCommand(uint16 cmd);
	void onAccept(S_DATA_HEAD& head, int32 ip, int32 port);
	void onDisconnect(S_DATA_HEAD& head, int32 ip, int32 port);
	void onRecv_SaveData(uint8* buf, S_UDP_BASE* c, const int32 recvBytes);
	

	int  recvData_kcp(char* buf, int32 recvBytes);
	void createKcp(int32 id);
	void updateKcp();
	void releaseKcp();
private:
	FSocket* UdpSocket;
	UDPClient_Thread* m_workthread;
	bool  isRuning;

	S_UDP_BASE  data;

	UDPCLIENTNOTIFY_EVENT      onAcceptEvent;
	UDPCLIENTNOTIFY_EVENT      onDisconnectEvent;
	UDPCLIENTNOTIFY_EVENT      onCommandtEvent;
};


extern TESTRPG_API UDPClient* __udp;