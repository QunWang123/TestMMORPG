
#include "TCPClient.h"
// #include "IPv4Address.h"
#include "IPAddress.h"
#include "Templates/SharedPointer.h"
#include "Networking/Public/Interfaces/IPv4/IPv4Address.h"

namespace func
{
	ConfigXML* __ClientInfo = nullptr;
}

namespace net
{
	//��ʼ���ͻ�������
	void S_SERVER_BASE::init(int sid)
	{
		ID = 0;
		serverID = sid;
		serverType = 0;
		recvBuf = (uint8*)FMemory::Malloc(func::__ClientInfo->RecvMax);
		sendBuf = (uint8*)FMemory::Malloc(func::__ClientInfo->SendMax);
		recvBuf_Temp = (uint8*)FMemory::Malloc(func::__ClientInfo->RecvOne);

		port = 13550;
		ip = "127.0.0.1";
		reset();
	}
	void S_SERVER_BASE::reset()
	{
		state = 0;
		rCode = func::__ClientInfo->RCode;
		recv_Head = 0;
		recv_Tail = 0;
		recv_TempHead = 0;
		recv_TempTail = 0;
		is_Recved = false;

		send_Head = 0;
		send_Tail = 0;
		send_TempTail = 0;
		is_Sending = false;
		is_SendCompleted = false;
		time_Heart = 0;
		time_AutoConnect = 0;

		FMemory::Memset(recvBuf, 0, func::__ClientInfo->RecvMax);
		FMemory::Memset(sendBuf, 0, func::__ClientInfo->SendMax);
		FMemory::Memset(recvBuf_Temp, 0, func::__ClientInfo->RecvOne);
	}

	TCPClient::TCPClient()
	{
		isPause = false;
		isFirstConnect = true;
		isRunning = false;
		socketfd = nullptr;

		onAcceptEvent = nullptr;
		onSecureEvent = nullptr;
		onDisconnectEvent = nullptr;
		onExceptEvent = nullptr;
		onCommand = nullptr;

		m_workthread = nullptr;
		socketfd = nullptr;
	}

	TCPClient::~TCPClient()
	{
		if (m_workthread != nullptr)
		{
			delete m_workthread;
			m_workthread = nullptr;
		}

		if (socketfd != nullptr)
		{
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(socketfd);
			socketfd = NULL;
		}

		m_data.reset();
	}

	/************************���ӷ�����******************************************/
	int32 TCPClient::initSocket()
	{
		if (socketfd != nullptr)
		{
			// PLATFORM_SOCKETSUBSYSTEM��ƥ�䵱ǰ��ʲôƽ̨��Ȼ�����ƽ̨����DestroySocket����
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(socketfd);
			socketfd = nullptr;
		}
		// NAME_Stream��ʽ�׽���
		socketfd = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("Fang"), false);
		return 0;
	}

	void TCPClient::resetIP(FString newip, int32 newport)
	{
		m_data.ip = newip;
		m_data.port = newport;
	}

	// ��ڣ������̡߳�
	// TCPClient_Thread�̳���FRunnable��ʵ�������Ĵ��麯��Run��
	// FRunnableThread::create�ֻ����Run��Run�ֵ�����TCPClient�����run
	// TCPClient�����run����һ��ѭ���ˣ�û���ӵ�ʱ�����ʼ��socket,���������ӣ������˾ͷ��Onrecv
	void TCPClient::runClient(int32 sid, FString ip, int32 port)
	{
		m_data.init(sid);
		m_data.time_AutoConnect = 0;
		m_data.ip = ip;
		m_data.port = port;

		isRunning = true;
		m_workthread = new TCPClient_Thread(this);
	}

	bool TCPClient::connectServer()
	{
		if (m_data.state >= func::C_CONNECT) return false;
		initSocket();
		if (socketfd == nullptr) return false;

		FIPv4Address ip;
		FIPv4Address::Parse(m_data.ip, ip);

		TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		addr->SetIp(ip.Value);
		addr->SetPort(m_data.port);

		bool isconnect = socketfd->Connect(*addr);
		if (isconnect)
		{
			socketfd->SetNoDelay(true);				// ����ngla�㷨����С���ͷ���������һ�𷢣���ʱ�Ժã����Դ�����
			socketfd->SetNonBlocking();
			m_data.state = func::C_CONNECT;
			m_data.time_HeartTime = 0;
			// if (onAcceptEvent != nullptr) this->onAcceptEvent(this, 0);
			return true;
		}
		return false;
	}

	void TCPClient::disconnectServer(const int32 errcode, FString err)
	{
		if (m_data.state == func::C_FREE) return;
		if (socketfd == nullptr) return;

		socketfd->Close();
		m_data.reset();

		if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, errcode);
	}

	/************************�շ�����******************************************/


	int32 TCPClient::onRecv()
	{
		if (socketfd == nullptr) return -1;
		FMemory::Memset(m_data.recvBuf_Temp, 0, func::__ClientInfo->RecvOne);

		uint32 size;
		// �ڹ����߳���������ж���û������
		if (socketfd->HasPendingData(size) == false) return -1;

		int32 recvBytes = 0;
		bool isrecv = socketfd->Recv(m_data.recvBuf_Temp, func::__ClientInfo->RecvOne, recvBytes);
		if (isrecv && recvBytes > 0)
		{
			auto c = this->getData();
			if (c->recv_Head == c->recv_Tail)
			{
				c->recv_Head = 0;
				c->recv_Tail = 0;	
			}

			if (c->recv_Tail + recvBytes >= func::__ClientInfo->RecvMax) return -1;
			
			FMemory::Memcpy(&c->recvBuf[c->recv_Tail], c->recvBuf_Temp, recvBytes);
			c->recv_Tail += recvBytes;
		}
		return 0;
	}

	int32 TCPClient::onSend()
	{
		auto c = this->getData();
		if (c->send_Tail <= c->send_Head) return 0;
		if (c->state < func::C_CONNECT) return -1;

		int32 sendLen = c->send_Tail - c->send_Head;
		if (sendLen < 1) return 0;

		// ʵ�ʷ����ģ�����ײ㷢�ͻ�����С��sendLen�Ļ��ͷ����꣬sendBytes��¼ʵ�ʷ���ȥ�ĳ���
		// ���Է����η������õ��ģ���Ϊc->send_Head += sendBytes;
		int32 sendBytes;
		bool issend = socketfd->Send(&c->sendBuf[c->send_Head], sendLen, sendBytes);

		if (issend && sendBytes > 0)
		{
			c->send_Head += sendBytes;
		}
		return 0;
	}
}