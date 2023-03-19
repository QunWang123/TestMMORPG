// Fill out your copyright notice in the Description page of Project Settings.


#include "UDPClient.h"
#include <string>
#include "MyGameInstance.h"
#include "ikcp.h"
using namespace func;
using namespace net;


TESTRPG_API UDPClient* __udp = NULL;
func::ConfigXML*  func::__UDPClientInfo = nullptr;

UDPClient::UDPClient()
{
	data.port = 0;
}

UDPClient::~UDPClient()
{
}
void UDPClient::runClient(const FString& ip, int32 port)
{
	data.init(func::__UDPClientInfo->RecvMax);
	data.strip = ip;
	data.port = port;
	data.ip = 0;

	InitSocket();
	//connectServer(true);
	runThread();
}
void UDPClient::resetIP(const FString& ip, int32 port)
{
	data.init(func::__UDPClientInfo->RecvMax);
	data.strip = ip;
	data.port = port;
	data.ip = 0;

	connectServer(true);
}
void UDPClient::closeClient()
{
	
}
void UDPClient::InitSocket()
{
	//FIPv4Address UdpAddr;
	//FIPv4Address::Parse("127.0.0.1", UdpAddr);
	//FIPv4Endpoint Endpoint(UdpAddr, 8001);//����8001�˿�
	//int32 BufferSize = 10240;//��������С�����ݾ����������
	////ʹ��ָ����NetID�Ͷ˿ڴ�������ʼ���µ�IPv4�˵㡣
	//FIPv4Endpoint Endpoint(FIPv4Address::Any, ThePort);  //����ip��ַ����

	int32 BufferSize = 1024 * 1024;

	FUdpSocketBuilder SocketBuilder = FUdpSocketBuilder(TEXT("UDPSocket"));
	//SocketBuilder.BoundToEndpoint(Endpoint);//�󶨵�ַ
	SocketBuilder.WithReceiveBufferSize(BufferSize);//���û�������С
	SocketBuilder.WithSendBufferSize(BufferSize);
	SocketBuilder.AsReusable();//ʹ�󶨵ĵ�ַ���Ա������׽������á�
	//SocketBuilder.AsNonBlocking();//������
	UdpSocket = SocketBuilder;

	//BUFFER SIZE
	UdpSocket->SetSendBufferSize(BufferSize, BufferSize);
	UdpSocket->SetReceiveBufferSize(BufferSize, BufferSize);
	UdpSocket->SetNonBlocking(false);
}

//127.0.0.1
//01110000  00000000 00000000 00000001
std::string INTtoIP(int32 num)
{

	std::string strRet = "";
	for (int i = 0; i < 4; i++)
	{
		uint32_t tmp = (num >> ((3 - i) * 8)) & 0xFF;

		char chBuf[8] = "";
		_itoa_s(tmp, chBuf, 10);
		strRet += chBuf;

		if (i < 3)
		{
			strRet += ".";
		}
	}

	return strRet;
}

void UDPClient::onAccept(S_DATA_HEAD& head, int32 ip, int32 port)
{
	if (data.ip != ip || data.port != port)
	{
		FString ss = FString::Printf(TEXT("onAccept err...%d/%d %d/%d"), data.ip, data.port, ip, port);
        if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);

		return;
	}
	if (head.ID < 0 || head.ID >= 10000)
	{
		FString ss = FString::Printf(TEXT("onAccept err %d"), head.ID);
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);
		return;
	}
	if (data.state == S_UConnect)
	{
		//LOG_MSG("�ͻ���������...%d\n", head.ID);
		return;
	}
	data.state = S_UConnect;
	data.ID = head.ID;
	data.time_Heart = __AppGameInstance->GetTimeSeconds();
	data.time_HeartConnect = __AppGameInstance->GetTimeSeconds();
	data.UdpSocket = this->UdpSocket;
	createKcp(head.ID);
	//FString ss = FString::Printf(TEXT("connect ok %d"), head.ID);
	//if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);

	//�ɷ����ӳɹ���Ϣ��ȥ
	if (onAcceptEvent != nullptr) this->onAcceptEvent(this, 0, "");
}
void UDPClient::onDisconnect(S_DATA_HEAD& head,int32 ip,int32 port)
{
	if (head.ID != func::__UDPClientInfo->Version)
	{
		FString ss = FString::Printf(TEXT("onDisconnect err %d"), head.ID, func::__UDPClientInfo->Version);
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);
		return;
	}
	if (data.ip != ip || data.port != port)
	{
		FString ss = FString::Printf(TEXT("onDisconnect err...%d/%d %d/%d"), data.ip, data.port, ip, port);
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);

		return;
	}

	//�ɷ����ӳɹ���Ϣ��ȥ
	if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, 65003, TEXT("server disconnect"));


}
void UDPClient::onRecv_SaveData(uint8* buf, S_UDP_BASE* c, const int32 recvBytes)
{
	//����BUF��������
	S_DATA_BASE base;
	base.reset();
	base.length = recvBytes + 4;
	memcpy(base.head, func::__UDPClientInfo->Head, 2);
	memcpy(base.buf, buf, MAX_UDP_BUF);

	if (c->recv_Head == c->recv_Tail)
	{
		c->recv_Tail = 0;
		c->recv_Head = 0;
	}
	//buff���������� ��������������
	if (c->recv_Tail + base.length >= func::__UDPClientInfo->RecvMax)
	{
		return;
	}

	memcpy(&c->recvBuf[c->recv_Tail], &base, base.length);
	c->recv_Tail += base.length;
	c->is_RecvCompleted = true;
}



//�������ݻص�
int UDPClient::recvData()
{
	TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	//ListenSocket->HasPendingData(Size) ��ѯ�׽�����ȷ���������Ƿ��й�������ݣ�����׽��������ݣ���Ϊtrue������Ϊfalse      
	//Size����ָʾ����recv���õĹܵ����ж�������

	uint32 size = 0;
	//if (UdpSocket->HasPendingData(size))
	{
		uint8 buf[512];
		int32 recvBytes = 0;
		FMemory::Memset(buf, 0, 512);
		bool isRecv = UdpSocket->RecvFrom(buf, 512, recvBytes, *targetAddr);
		if (!isRecv) return recvBytes;
		if (recvBytes < 6) return recvBytes;

		//��ȡͷ���� ����ID
		S_DATA_HEAD head;
		memcpy(&head, buf, sizeof(S_DATA_HEAD));
		head.getSecure(func::__UDPClientInfo->RCode);



		//��������
		switch (head.cmd)
		{
		case CMD_65001://���ӳɹ�
			{
				uint32 ip = 0;
				int32 port = targetAddr->GetPort();
				targetAddr->GetIp(ip);
				onAccept(head, ip, port);
			}
			return recvBytes;
		case CMD_65003://�Ͽ�����
			{
				uint32 ip = 0;
				int32 port = targetAddr->GetPort();
				targetAddr->GetIp(ip);
				onDisconnect(head, ip, port);
			}
			return recvBytes;
		}
		//1 ���ID��� ˵���Ǵ�UDP���ͨ�� 
		if (head.ID == data.ID)
		{
			onRecv_SaveData(buf, &data, recvBytes);
			return recvBytes;
		}

		//2 KCP��������
		recvData_kcp((char*)buf, recvBytes);

		//onRecv_SaveData(buf, &data, recvBytes);
		//FString debugData = UTF8_TO_TCHAR(buf);   //�ַ���ת��
		//TArray<uint8> arr = targetAddr->GetRawIp();
		//std::string str;
		//for (int i = 0; i < arr.Num(); i++)
		//{
		//	if (i == 0)
		//		str = std::to_string(arr[i]);
		//	else
		//		str = str + "." + std::to_string(data[i]);
		//}

		//FString ssip = UTF8_TO_TCHAR(str.c_str());
		//int32 port = targetAddr->GetPort();

		//FString ss = FString::Printf(TEXT("recvdata... %s [ip:%s/%d]"), *debugData,*ssip,port);
		//if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);

		return recvBytes;
	}
	
	return -1;
}



void UDPClient::sendData(const void* buf,const int32 size, const FString& ip,const int32 port)
{
	static TSharedPtr<FInternetAddr>	addr;
	if (!addr.IsValid())
	{
		addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	}
	bool isValid = false;
	addr->SetIp(*ip, isValid);
	addr->SetPort(port);
	int32 sendBytes = 0;
	UdpSocket->SendTo((uint8*)buf, size, sendBytes, *addr);
}

void UDPClient::sendData(const void* buf, const int32 size, const int8 protocolType)
{
	if (protocolType == SPT_KCP)
	{
		if (data.kcp == NULL) return;
		ikcp_send(data.kcp, (char*)buf, size);
		return;
	}


	static TSharedPtr<FInternetAddr>	addrserver;
	if (!addrserver.IsValid())
	{
		addrserver = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	}
	bool isValid = false;
	addrserver->SetIp(*(this->data.strip), isValid);
	addrserver->SetPort(this->data.port);

	if (data.ip == 0)
	{
		addrserver->GetIp(data.ip);
	}

	int32 sendBytes = 0;
	UdpSocket->SendTo((uint8*)buf, size, sendBytes, *addrserver);
}

