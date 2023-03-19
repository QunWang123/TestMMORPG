#include "UdpServer.h"
#ifdef ___WIN32_
#include <WS2tcpip.h>
#endif
#include "TcpServer.h"
using namespace func;


namespace net
{
	IUdpServer* NewUdpServer()
	{
		return new UdpServer();
	}


	UdpServer::UdpServer()
	{
		socketfd = -1;
		Linkers = NULL;
	}

	UdpServer::~UdpServer()
	{
	}

	//����������
	int UdpServer::runServer()
	{
		//1�����������û�
		Linkers = new HashArray<S_UDP_BASE>(func::__UDPServerInfo->MaxConnect);
		for (int i = 0; i < Linkers->length; i++)
		{
			S_UDP_BASE* client = Linkers->Value(i);
			client->init(func::__UDPServerInfo->RecvMax);
		}
		initCommands();
		//��ʼ��socket
		initSocket();
		//�����߳�
		runThread();
		return 0;
	}

	void UdpServer::stopServer()
	{
	}



	int UdpServer::initSocket()
	{
		//1 ����UDP socket
		socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (socketfd < 0)
		{
			perror("socket error��");
			exit(1);
		}
		//����SOCKETΪ������
		func::setNonblockingSocket(false, socketfd);

//#ifdef ____WIN32__
//		BOOL bNewBehavior = FALSE;
//		DWORD dwBytesReturned = 0;
//		WSAIoctl(socketfd, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
//#endif
		//2 �����������ַk
		struct sockaddr_in addserver;
		addserver.sin_family = AF_INET;
		addserver.sin_addr.s_addr = htonl(INADDR_ANY);
		addserver.sin_port = htons(__UDPServerInfo->Port);

		//3 �󶨵�ַ 
		int err = bind(socketfd, (struct sockaddr*)&(addserver), sizeof(addserver));
		if (err < 0)
		{
			perror("bind");
			exit(1);
		}

		//int bufferSize = 10240;
		//setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char*)&bufferSize, sizeof(int));
		//setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (char*)&bufferSize, sizeof(int));

		int recvSize = 0;
		int sendSize = 0;
		//socklen_t optlen = sizeof(recvSize);
		//getsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (char*)&recvSize, &optlen);
		//getsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char*)&sendSize, &optlen);
		

		printf("udp server suceess...%d/%d \n", sendSize, recvSize);


	}

	//�����µ�����
	void UdpServer::onAccept(char* buf, S_DATA_HEAD& head, sockaddr_in& clientAddr)
	{
		//1 ���汾��
		//if (head.ID != func::__UDPServerInfo->Version)
		//{
		//	printf("�������Ӱ汾�Ŵ���... %d err:%d\n", head.ID, func::getError());
		//	return;
		//}

		//��ȡIP �˿�
		std::string strip = inet_ntoa(clientAddr.sin_addr);
		u32 ip = clientAddr.sin_addr.S_un.S_addr;// inet_addr(strip.c_str());
		int port = ntohs(clientAddr.sin_port);
		
		//2�����IP�Ͷ˿ڶ�Ӧ������ �ǲ��Ǵ��ڣ�
		if(checkIsConnect(ip,port))
		{
			LOG_MESSAGE("udp ��ǰ�����Ѵ���...[%s:%d:%d]\n", strip.c_str(), ip, port);

			//��������ID
			S_DATA_HEAD  d;
			d.ID = head.ID;
			d.cmd = CMD_65001;
			d.setSecure(func::__UDPServerInfo->RCode);
			sendData(head.ID, &d, sizeof(S_DATA_HEAD), net::SPT_UDP);
			return;
		}
		//3��ƥ��һ�����е�����
		S_UDP_BASE* client = findClient(head.ID);// findFree();
		if (client == NULL)
		{
			LOG_MESSAGE("��ǰ��������������...[%s:%d:%d]\n", strip.c_str(), ip, port);
			return;
		}


		//��������
		client->ID = head.ID;
		client->socketfd = socketfd;
		client->state = S_UConnect;//����
		client->port = port;
		client->ip = ip;
		client->time_Heart= time(NULL);
		client->time_HeartConnect = time(NULL);
		memcpy(client->strip, strip.c_str(), 16);
		memcpy(&client->addr, &clientAddr, sizeof(sockaddr_in));
		addCheck(ip, port, client->ID);
		createKcp(client);
		//��������3�����ݰ����ͻ���
		for (int i = 0; i < 3; i++)
		{
			//��������ID
			S_DATA_HEAD  d;
			d.ID = client->ID;
			d.cmd = CMD_65001;
			d.setSecure(func::__UDPServerInfo->RCode);
			sendData(client->ID, &d, sizeof(S_DATA_HEAD),net::SPT_UDP);
		}
	
		//�ɷ����ӳɹ���Ϣ��ȥ
		if (onAcceptEvent != nullptr) this->onAcceptEvent(this, client, 0,"");
	}

	//����buf���ݵ�������
	void UdpServer::onRecv_SaveData(char* buf, S_UDP_BASE* c,const u32 recvBytes)
	{
		//����BUF��������
		func::S_DATA_BASE base;
		base.reset();
		base.length = recvBytes + 4;
		memcpy(base.head, func::__UDPServerInfo->Head, 2);
		memcpy(base.buf, buf, MAX_UDP_BUF);

		if (c->recv_Head == c->recv_Tail)
		{
			c->recv_Tail = 0;
			c->recv_Head = 0;
		}
		//buff���������� ��������������
		if (c->recv_Tail + base.length >= func::__UDPServerInfo->RecvMax)
		{
			LOG_MESSAGE("buff��������...%d\n", c->ID);
			return;
		}

		memcpy(&c->recvBuf[c->recv_Tail], &base, base.length);
		c->recv_Tail += base.length;
		c->is_RecvCompleted = true;
	}

	//����udp����
	int UdpServer::recvData()
	{
		//printf("recvData***************************\n");
		//576 - 20 - 8 = 548
		char buf[MAX_UDP_BUF];
		memset(buf, 0, MAX_UDP_BUF);

		sockaddr_in clientAddr;
		socklen_t len = sizeof(clientAddr);
		//����
		//������
		int recvBytes = recvfrom(socketfd, buf, 512, 0, (struct sockaddr*)&clientAddr, &len);
		if (recvBytes < 6)
		{
			//10054���� �Զ˲�����
			int err = func::getError();
			if (err == 10054)
			{
				int port = ntohs(clientAddr.sin_port);
				u32 ip = clientAddr.sin_addr.S_un.S_addr;
				u64 key = (u64)ip * MAX_VALUE + port;
				auto it = m_CheckConnect.find(key);
				if (it != m_CheckConnect.end())
				{
					s32 id = it->second;
					auto c = findClient(id, ip, port);
					if (c != NULL)
					{
						if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, c, 10054, "client not exist");
					}
				}
			}
			else
			   LOG_MESSAGE("recvData err...%d err:%d\n", recvBytes, err);
			return recvBytes;
		}

		//��ȡͷ���� ����ID
		S_DATA_HEAD head;
		memcpy(&head, buf, sizeof(S_DATA_HEAD));
		
		head.getSecure(func::__UDPServerInfo->RCode);
		LOG_MESSAGE("OnCmd_%d\n", head.cmd);
		//��������
		switch (head.cmd)
		{
		case CMD_65001://��������
			onAccept(buf, head,clientAddr);
			return recvBytes;
		}
		
		int port = ntohs(clientAddr.sin_port);
		u32 ip = clientAddr.sin_addr.S_un.S_addr;
		//LOG_MSG("recvData...ID:%d cmd:%d \n", head.ID, head.cmd);

		//1 ��UDPͨ�ſ��
		auto client = findClient(head.ID, ip, port);
		if (client == NULL)
		{
			//��� �������� ���ؿͻ��� ��Ҫ��������
			if (head.cmd == CMD_65002)
			{
				LOG_MESSAGE("recvData 65002 findClient=NULL %d %d %d\n", head.ID, ip, port);

				bool issend = checkIsSendTime(ip, port);
				if (issend == false) return recvBytes;

				std::string strip = inet_ntoa(clientAddr.sin_addr);
				//��������ID
				S_DATA_HEAD  d;
				d.cmd = CMD_65003;
				d.ID = func::__UDPServerInfo->Version;
				d.setSecure(func::__UDPServerInfo->RCode);
				
				sendData(&d, sizeof(S_DATA_HEAD),strip.c_str(),port);

				addCheckSendTime(ip, port);
				return recvBytes;
			}
		}
		else if (client != NULL)
		{
			//�������ݵ����߳� ������� ���ܸ�Ч
			onRecv_SaveData(buf, client, recvBytes);
			return recvBytes;
		}

		//2 KCP��������
		recvData_kcp(buf, recvBytes,ip,port);

		return recvBytes;
	}

	int UdpServer::sendData(const void* buf, const u32 size, const char* ip, const int port)
	{
		sockaddr_in clientAddr;
		clientAddr.sin_family = AF_INET;
		clientAddr.sin_addr.s_addr = inet_addr(ip);
		clientAddr.sin_port = htons(port);

		int sendBytes = sendto(socketfd, (char*)buf, size, 0, (struct sockaddr*) & clientAddr, sizeof(struct sockaddr_in));
	
		if (sendBytes != size)
		{
			printf("sendData err...%d err:%d \n", sendBytes, func::getError());
			return -1;
		}
		//printf("sendData successfull...%d \n", sendBytes);
		return sendBytes;
	}



	int UdpServer::sendData(const s32 id, const void* buf, const u32 size, const u8 protocolType)
	{
		auto c = findClient(id, S_UConnect);
		if (c == NULL) return -1;

		if (protocolType == SPT_UDP)
		{
			return sendto(socketfd, (char*)buf, size, 0, (struct sockaddr*)&c->addr, sizeof(struct sockaddr_in));
		}
		else if (protocolType == SPT_KCP)
		{
			return ikcp_send(c->kcp, (char*)buf, size);
		}
	}
}

