#include "UdpClient.h"
#include <fcntl.h>
#ifdef ___WIN32_
#include <WS2tcpip.h>
#endif

namespace net
{

	IUdpClient* NewUdpClient()
	{
		return new UdpClient();
	}

	UdpClient::UdpClient()
	{
		socketfd = -1;
	}

	UdpClient::~UdpClient()
	{
	}


	void UdpClient::stopClient()
	{
	}

	void UdpClient::setAddress(char* ip, int port)
	{
		data.addr.sin_family = AF_INET;
		data.addr.sin_addr.s_addr = inet_addr(ip);
		data.addr.sin_port = htons(port);

		data.ip = inet_addr(ip);
		data.port = port;
		strcpy(data.strip, ip);
		
	}
	//启动服务器
	int UdpClient::runClient(char* ip, u16 port,u32 id)
	{
		//初始化 设置地址
		data.init(func::__UDPClientInfo->RecvMax);
		data.time_AutoConnect = 0;
		setAddress(ip, port);
		data.threadID = id;
	

		initCommands();
		//初始化socket
		initSocket();
		connectServer();
		//启动线程
		runThread();
		return 0;
	}

	int UdpClient::initSocket()
	{
		//1 创建UDP socket
		socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (socketfd < 0)
		{
			perror("socket error！");
			exit(1);
		}
		//设置SOCKET为非阻塞
		func::setNonblockingSocket(false,socketfd);
//#ifdef ____WIN32__
//		BOOL bNewBehavior = FALSE;
//		DWORD dwBytesReturned = 0;
//		WSAIoctl(socketfd, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
//#endif


		printf("udp client suceess...\n");
	}

	void UdpClient::onAccept(S_DATA_HEAD& head, sockaddr_in& addr)
	{
		u16 port = ntohs(addr.sin_port);
		u32 ip = addr.sin_addr.S_un.S_addr;
		if (data.ip != ip || data.port != port)
		{
			LOG_MESSAGE("建立连接但是IP端口号不一致...%d/%d %d/%d\n", data.ip, data.port, ip, port);
			return;
		}
		if (head.ID < 0 || head.ID >= func::__UDPClientInfo->MaxConnect)
		{
			LOG_MESSAGE("建立连接获取索引越界...%d\n", head.ID);
			return;
		}
		if(data.state == S_UConnect)
		{
			//LOG_MSG("客户端已连接...%d\n", head.ID);
			return;
		}
		//设置数据
		data.state = S_UConnect;
		data.ID = head.ID;
		data.time_Heart = time(NULL);
		data.socketfd = this->socketfd;
		createKcp(head.ID);
		
		//派发连接成功信息出去
		if (onAcceptEvent != nullptr) this->onAcceptEvent(this,  0, "");
	}


	void UdpClient::onDisconnect(S_DATA_HEAD& head, sockaddr_in& addr)
	{
		if (head.ID != func::__UDPClientInfo->Version)
		{
			LOG_MESSAGE("版本号不一致...%d/%d \n", head.ID, func::__UDPClientInfo->Version);
			return;
		}
		u16 port = ntohs(addr.sin_port);
		u32 ip = addr.sin_addr.S_un.S_addr;
		if (data.ip != ip || data.port != port)
		{
			LOG_MESSAGE("IP端口号不一致...%d/%d %d/%d\n", data.ip, data.port, ip, port);
			return;
		}

		
		//派发连接成功信息出去
		if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, 65003, "server disconnect");

	
	}
	void UdpClient::onRecv_SaveData(char* buf, S_UDP_BASE* c, const u32 recvBytes)
	{
		//保存BUF缓存数据
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
		//buff缓冲区已满 抛弃掉这条数据
		if (c->recv_Tail + base.length >= func::__UDPServerInfo->RecvMax)
		{
			LOG_MESSAGE("buff缓冲区满...%d\n", c->ID);
			return;
		}

		memcpy(&c->recvBuf[c->recv_Tail], &base, base.length);
		c->recv_Tail += base.length;
		c->is_RecvCompleted = true;
	}
	//接收udp数据
	int UdpClient::recvData()
	{
		//printf("recvData***************************\n");

		char buf[512];
		memset(buf, 0, 512);
		sockaddr_in clientAddr;
		socklen_t len = sizeof(clientAddr);
		int recvBytes = recvfrom(socketfd, buf, 512, 0, (struct sockaddr*)&clientAddr, &len);
		if (recvBytes < 6)
		{
			int err = func::getError();
			if (err == 10054)
			{
				if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this,10054, "server not open");
			}
			else
				LOG_MESSAGE("recvData err...%d err:%d\n", recvBytes, err);
			return recvBytes;
		}

		//获取头数据 解析ID
		S_DATA_HEAD head;
		memcpy(&head, buf, sizeof(S_DATA_HEAD));
		head.getSecure(func::__UDPClientInfo->RCode);

		//接受连接
		switch (head.cmd)
		{
		case CMD_65001://连接成功
			onAccept(head, clientAddr);
			return recvBytes;
		case CMD_65003://断开连接
			onDisconnect(head, clientAddr);
			return recvBytes;
		}

		//1 如果ID相等 说明是纯UDP框架通信 
		if (head.ID == data.ID)
		{
			onRecv_SaveData(buf, &data, recvBytes);
			return recvBytes;
		}

		//2 KCP解析数据
		recvData_kcp(buf, recvBytes);



	/*	std::string ip = inet_ntoa(clientAddr.sin_addr);
		int16_t port = ntohs(clientAddr.sin_port);
		printf("recvData...OK [%s]  recvBytes:%d [%s:%d] \n", buf, recvBytes, ip.c_str(), port);*/
		
		//printf("recData ID:%d cmd:%d \m", head.ID, head.cmd);
		return recvBytes;
	}
	//int UdpClient::sendData_kcp(void* buf, u32 size)
	//{
	//	return ikcp_send(data.kcp, (char*)buf, size);
	//}

	int UdpClient::sendData(const char* buf, const u32 size, const char* ip, const int port)
	{
		sockaddr_in clientAddr;
		clientAddr.sin_family = AF_INET;
		clientAddr.sin_addr.s_addr = inet_addr(ip);
		clientAddr.sin_port = htons(port);

		int sendBytes = sendto(socketfd, buf, size, 0, (struct sockaddr*)&clientAddr, sizeof(struct sockaddr_in));

		if (sendBytes != size)
		{
			printf("sendData err...%d err:%d \n", sendBytes, func::getError());
			return -1;
		}
		//printf("sendData successfull...%d \n", sendBytes);
		return sendBytes;
	}

	int UdpClient::sendData(const void* buf, const u32 size, const u8 protocolType)
	{
		if (protocolType == SPT_UDP)
		{
			return sendto(socketfd, (char*)buf, size, 0, (struct sockaddr*)&data.addr, sizeof(struct sockaddr_in));
		}
		else if (protocolType == SPT_KCP)
		{
			return ikcp_send(data.kcp, (char*)buf, size);
		}
	}





	S_UDP_BASE* UdpClient::getData()
	{
		return &data;
	}
	//***********************************************

	

}

