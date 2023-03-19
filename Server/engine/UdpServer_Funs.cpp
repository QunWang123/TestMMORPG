#include "UdpServer.h"
using namespace func;


//业务层指令集合
std::vector<IUDPContainer*> __CommandsUDP;

namespace net
{
	//******************************************************************
	void  UdpServer::initCommands()
	{
		__CommandsUDP.reserve(MAX_COMMAND_LEN);
		for (int i = 0; i < MAX_COMMAND_LEN; i++)
		{
			__CommandsUDP.emplace_back(nullptr);
		}
	}
	//注册
	void  UdpServer::registerCommand(int cmd, void* container)
	{
		if (cmd >= MAX_COMMAND_LEN) return;
		IUDPContainer* icon = (IUDPContainer*)container;
		if (icon == nullptr) return;
		__CommandsUDP[cmd] = icon;
	}

	void UdpServer::setOnUdpClientAccept(UDPSERVERNOTIFY_EVENT event)
	{
		onAcceptEvent = event;
	}

	void UdpServer::setOnUdpClientDisconnect(UDPSERVERNOTIFY_EVENT event)
	{
		onDisconnectEvent = event;
	}

	//******************************************************************

	void UdpServer::parseCommand()
	{
		for (u32 i = 0; i < Linkers->length; i++)
		{
			auto c = Linkers->Value(i);
			if (c->ID < 0) continue;
			if (c->state == S_UFree) continue;

			checkConnect(c);
			if (c->state == S_UFree) continue;
			updateKcp(c);
			parseCommand(c);
		}
	}




	void UdpServer::parseCommand(S_UDP_BASE* c)
	{
		if (!c->is_RecvCompleted) return;

		while (c->recv_Tail - c->recv_Head >= 10)
		{
			//1、解析头
			char head[2];
			head[0] = c->recvBuf[c->recv_Head];
			head[1] = c->recvBuf[c->recv_Head + 1];

			if (head[0] != func::__UDPServerInfo->Head[0] || head[1] != func::__UDPServerInfo->Head[1])
			{
				LOG_MESSAGE("解析头出错...\n");
				return;
			}
			u16 length = (*(u16*)(c->recvBuf + c->recv_Head + 2));
			//u16 cmd = (*(u16*)(c->recvBuf + c->recv_Head + 8));
			//cmd = cmd ^ func::__UDPServerInfo->RCode;
			S_DATA_HEAD data;
			memcpy(&data, &c->recvBuf[c->recv_Head+4], sizeof(S_DATA_HEAD));
			data.getSecure(func::__UDPServerInfo->RCode);

			//2、长度不够 需要继续等待 
			if (c->recv_Tail < c->recv_Head + length)
			{
				LOG_MESSAGE("解析数据长度出错...%d %d\n", c->recv_Tail, c->recv_Head + length);
				return;
			}

			c->recv_TempHead = c->recv_Head + 4;
			c->recv_TempTail = c->recv_Head + length;

			parseCommand(c, data.cmd);
			//4、增加读取长度
			c->recv_Head += length;

		}

		c->is_RecvCompleted = false;
	}
	void UdpServer::parseCommand(S_UDP_BASE* c, const u16 cmd)
	{
		c->time_HeartConnect = (int)time(NULL);


		switch (cmd)
		{
		case CMD_65002://HEART
			{
				char s[50];
				func::formatTime(time(NULL), s);
				//LOG_MSG("heart %s...%d [%s:%d]\n", s, c->ID, c->strip, c->port);
				//sendHeart(c);
			}
			return;
		case CMD_65003: //客户端关闭
			if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, c, 65003, "client close");
			return;
		}

		auto container = __CommandsUDP[cmd];
		if (container == nullptr)
		{
			LOG_MESSAGE("command not register...%d \n", cmd);
			return;
		}

		//触发事件
		container->onUDPServerCommand(this, c, cmd);
	}





	void UdpServer::checkConnect(S_UDP_BASE* c)
	{
		s32 temp = (s32)time(NULL) - c->time_HeartConnect;
		if (temp < func::__UDPServerInfo->HeartTimeMax) return;
		
		if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, c, 1001, "heart timeourt");
	}

	S_UDP_BASE* UdpServer::findFree()
	{
		for (int i = 0; i < Linkers->length; i++)
		{
			S_UDP_BASE* client = Linkers->Value(i);
			if (client->state != S_UFree) continue;

			client->reset();
			client->ID = i;
			return client;

		}

		return NULL;
	}
	bool UdpServer::checkIsConnect(u32 ip, u16 port)
	{
		u64 key = (u64)ip * MAX_VALUE + port;
		auto it = m_CheckConnect.find(key);
		if (it == m_CheckConnect.end())
		{
			return false;
		}

		return true;
	}
	//增加检查
	void UdpServer::addCheck(u32 ip, u16 port, s32 id)
	{
		u64 key = (u64)ip * MAX_VALUE + port;
		{
			std::lock_guard<std::mutex> guard(this->m_CheckConnectMutex);
			m_CheckConnect.insert(std::make_pair(key, id));
		}

	}
	//删除检查
	void UdpServer::deleteCheck(u32 ip, u16 port)
	{
		u64 key = (u64)ip * MAX_VALUE + port;
		{
			std::lock_guard<std::mutex> guard(this->m_CheckConnectMutex);
			auto it = m_CheckConnect.find(key);
			if (it == m_CheckConnect.end()) return;
			m_CheckConnect.erase(it);
		}


	}
	bool UdpServer::checkIsSendTime(u32 ip, u16 port)
	{
		u64 key = (u64)ip * MAX_VALUE + port;
		auto it = m_CheckSendTime.find(key);
		if (it == m_CheckSendTime.end())
		{
			return true;
		}
		int value = it->second;
		value = time(NULL) - value;
		if (value < 3) return false;

		return true;
	}
	void UdpServer::addCheckSendTime(u32 ip, u16 port)
	{
		u64 key = (u64)ip * MAX_VALUE + port;
		m_CheckSendTime.insert(std::make_pair(key, time(NULL)));
	}
	//发送心跳包
	void UdpServer::sendHeart(S_UDP_BASE* c)
	{
		s32 temp = (s32)time(NULL) - c->time_Heart;
		if (temp < func::__UDPClientInfo->HeartTime) return;
		c->time_Heart = time(NULL);

		LOG_MESSAGE("sendHeart...%d \n", c->ID);
		//返回连接ID
		S_DATA_HEAD  d;
		d.cmd = CMD_65002;
		d.ID =c->ID;
		d.setSecure(func::__UDPServerInfo->RCode);
		sendData(&d, sizeof(S_DATA_HEAD), c->strip,c->port);
	}
	S_UDP_BASE* UdpServer::findClient(const int id)
	{
		if (id < 0 || id >= Linkers->length) return nullptr;

		S_UDP_BASE* c = Linkers->Value(id);
		return c;
	}

	S_UDP_BASE* UdpServer::findClient(const int id, int state)
	{
		if (id < 0 || id >= Linkers->length) return nullptr;

		S_UDP_BASE* c = Linkers->Value(id);
		if (c->state == state) return c;
		return nullptr;
	}
	S_UDP_BASE* UdpServer::findClient(const int id, u32 ip, u16 port)
	{
		if (id < 0 || id >= Linkers->length) return nullptr;

		S_UDP_BASE* c = Linkers->Value(id);
		if (c->state != S_UConnect) return nullptr;
		if (c->ip != ip || c->port != port) return nullptr;
		return c;
	}

	int UdpServer::getConnectCount()
	{
		return m_CheckConnect.size();
	}
	//清理连接数据
	void UdpServer::clearConnect(S_UDP_BASE* c)
	{
		deleteCheck(c->ip, c->port);
		releaseKcp(c);
		c->reset();
	}


	void UdpServer::readData(const int id, void* v, const u32 len)
	{
		auto c = findClient(id);
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
}