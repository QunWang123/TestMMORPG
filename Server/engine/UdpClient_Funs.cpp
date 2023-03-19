#include "UdpClient.h"
#include "INetBase.h"
#include "IContainer.h"
using namespace func;


namespace net
{
	//业务层指令集合
	std::vector<IUDPContainer*> __Commands;

	//******************************************************************
	void  UdpClient::initCommands()
	{
		__Commands.reserve(MAX_COMMAND_LEN);
		for (int i = 0; i < MAX_COMMAND_LEN; i++)
		{
			__Commands.emplace_back(nullptr);
		}
	}
	//注册
	void  UdpClient::registerCommand(int cmd, void* container)
	{
		if (cmd >= MAX_COMMAND_LEN) return;
		IUDPContainer* icon = (IUDPContainer*)container;
		if (icon == nullptr) return;
		__Commands[cmd] = icon;
	}

	void UdpClient::setOnUdpServerAccept(UDPCLIENTNOTIFY_EVENT event)
	{
		onAcceptEvent = event;
	}

	void UdpClient::setOnUdpServerDisconnect(UDPCLIENTNOTIFY_EVENT event)
	{
		onDisconnectEvent = event;
	}

	void UdpClient::parseCommand()
	{
		if (data.state == S_UFree)
		{
			connectServer();
			return;
		}
		checkConnect();
		if (data.state == S_UFree) return;

		S_DATA_HEAD  d;
		d.ID = data.ID;
		d.cmd = 3000;
		d.setSecure(func::__UDPClientInfo->RCode);
		sendData(&d, sizeof(S_DATA_HEAD), net::SPT_KCP);

		sendHeart();
		updateKcp();

		auto c = getData();
		while (c->recv_Tail - c->recv_Head >= 10)
		{
			//1、解析头
			char head[2];
			head[0] = c->recvBuf[c->recv_Head];
			head[1] = c->recvBuf[c->recv_Head + 1];

			if (head[0] != func::__UDPClientInfo->Head[0] || head[1] != func::__UDPClientInfo->Head[1])
			{
				LOG_MESSAGE("解析头出错...\n");
				return;
			}
			u16 length = (*(u16*)(c->recvBuf + c->recv_Head + 2));

			S_DATA_HEAD data;
			memcpy(&data, &c->recvBuf[c->recv_Head + 4], sizeof(S_DATA_HEAD));
			data.getSecure(func::__UDPClientInfo->RCode);

			//2、长度不够 需要继续等待 
			if (c->recv_Tail < c->recv_Head + length)
			{
				LOG_MESSAGE("解析数据长度出错...%d %d\n", c->recv_Tail, c->recv_Head + length);
				return;
			}
			c->recv_TempHead = c->recv_Head + 4;
			c->recv_TempTail = c->recv_Head + length;
			parseCommand(data.cmd);
			//4、增加读取长度
			c->recv_Head += length;
		}
	}

	void UdpClient::parseCommand(u16 cmd)
	{
		data.time_HeartConnect = time(NULL);

		switch (cmd)
		{
		case CMD_65002:
				return;
		}

		auto container = __Commands[cmd];
		if (container == nullptr)
		{
			LOG_MESSAGE("command not register...%d \n", cmd);
			return;
		}

		//触发事件
		container->onUDPClientCommand(this, cmd);
	}
	void UdpClient::checkConnect()
	{
		s32 temp = (s32)time(NULL) - data.time_HeartConnect;
		if (temp < func::__UDPClientInfo->HeartTimeMax) return;

		if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, 1001, "heart timeout");
	}


	void UdpClient::connectServer()
	{
		if (data.state == S_UConnect) return;

		int tempTime = (int)time(NULL) - data.time_AutoConnect;
		if (tempTime < func::__UDPClientInfo->AutoTime) return;
		data.time_AutoConnect = (int)time(NULL);

		S_DATA_HEAD d;
		// d.ID = func::__UDPClientInfo->Version;
		d.ID = 0;
		d.cmd = CMD_65001;
		d.setSecure(func::__UDPClientInfo->RCode);
		sendData(&d, sizeof(S_DATA_HEAD),net::SPT_UDP);

		LOG_MESSAGE("connectServer...\n");
	}



	void UdpClient::sendHeart()
	{
		s32 temp = (s32)time(NULL) - data.time_Heart;
		if (temp < func::__UDPClientInfo->HeartTime) return;

		data.time_Heart = time(NULL);
		//返回连接ID
		S_DATA_HEAD  d;
		d.ID = data.ID;
		d.cmd = CMD_65002;
		d.setSecure(func::__UDPClientInfo->RCode);
		sendData(&d, sizeof(S_DATA_HEAD),net::SPT_UDP);

		LOG_MESSAGE("sendHeart...%d \n", data.ID);
	}
	void UdpClient::readData(void* v, const u32 len)
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
	void UdpClient::clearConnect()
	{
		releaseKcp();
		this->data.initdata();
	}
}