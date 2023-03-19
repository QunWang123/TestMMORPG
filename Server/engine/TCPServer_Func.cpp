#include "TcpServer.h"
#ifdef ___WIN32_

#include "IOPool.h"
using namespace net;
using namespace func;

//ҵ���ָ���
std::vector<IContainer*> __Commands;
//******************************************************************
void  TCPServer::initCommands()
{
	__Commands.reserve(MAX_COMMAND_LEN);
	for (int i = 0; i < MAX_COMMAND_LEN; i++)
	{
		__Commands.emplace_back(nullptr);
	}
}
//ע��
void  TCPServer::registerCommands(int cmd, void* container)
{
	if (cmd >= MAX_COMMAND_LEN) return;
	IContainer * icon = (IContainer*)container;
	if (icon == nullptr) return;
	__Commands[cmd] = icon;
}
//******************************************************************


//���߳��� ��������
void TCPServer::parseCommand()
{
	for (s32 i = 0; i < Linkers->length; i++)
	{
		auto c = Linkers->Value(i);
		if (c->ID == -1) continue;
		if (c->state == func::S_FREE) continue;
		if (c->state == func::S_NEEDSAVE) continue;

		checkConnect(c);
		if (c->closeState == func::S_CLOSE_SHUTDOWN) continue;
		parseCommand(c);
		
		// ���ڲ鿴���û��ķ��ͻ��������ǲ����������ݣ��������ݾ�popSendContext(������Ϊ�ж��Ƿ񷢳�)��Ȼ��������
		this->postSend(c);
	}
}
void net::TCPServer::getSecurityCount(int& connum, int& securtiynum)
{
	connum = m_ConnectCount;
	securtiynum = m_SecurityCount;
}
//������ ��������
void  TCPServer::parseCommand(S_CLIENT_BASE* c)
{
	// ��ʼ��Ϊfalse,�������ݷ����û��Ļ�����ɺ�����Ϊtrue,��������ִ�н���������
	if (!c->is_RecvCompleted) return;

	while (c->recv_Tail - c->recv_Head > 7)
	{
		//1������ͷ
		char head[2];
		head[0] = c->recvBuf[c->recv_Head] ^ c->rCode;
		head[1] = c->recvBuf[c->recv_Head + 1] ^ c->rCode;

		if (head[0] != __ServerInfo->Head[0] || head[1] != __ServerInfo->Head[1])
		{
			shutDown(c->socketfd, 0, c, 2001);
			return;
		}

		s32 length = (*(u32*)(c->recvBuf + c->recv_Head + 2)) ^ c->rCode;
		u16 cmd = (*(u16*)(c->recvBuf + c->recv_Head + 6)) ^ c->rCode;

		//2�����Ȳ��� ��Ҫ�����ȴ� 
		if (c->recv_Tail < c->recv_Head + length) break;

		c->recv_TempHead = c->recv_Head + 8;
		c->recv_TempTail = c->recv_Head + length;

		parseCommand(c, cmd);

		if (c->state <= func::S_CONNECT)
		{
			LOG_MESSAGE("clinet�Ѿ�reset....\n");
			return;
		}
		//4�����Ӷ�ȡ����
		c->recv_Head += length;

	}

	c->is_RecvCompleted = false;
}
//������ϸͷָ��
void net::TCPServer::parseCommand(S_CLIENT_BASE* c, u16 cmd)
{
	if (cmd < 65000)
	{
		if (cmd == CMD_HEART)
		{
			// c->time_Heart = (int)time(NULL);
			u32 value = 0;
			read(c->ID, value);

			begin(c->ID, CMD_HEART);
			sss(c->ID, value);
			end(c->ID);
			c->time_Heart = (int)time(NULL);
			if (func::__ServerInfo->Type != func::S_TYPE_GATE) return;
			// return;
			//LOG_MESSAGE("recv CMD_HEART...%d \n", value);
		}

		auto container = __Commands[cmd];
		if (container == nullptr)
		{
			LOG_MESSAGE("command not register...%d \n", cmd);
			return;
		}

		//�����¼�
		container->onServerCommand(this, c, cmd);
		return;
	}
	switch (cmd)
	{
	case CMD_SECURITY://��ȫ����
		char a[20];
		sprintf_s(a, "%s_%d", __ServerInfo->SafeCode, c->rCode);
		memset(c->md5, 0, sizeof(c->md5));

		if (func::MD5str != NULL) func::MD5str(c->md5, (unsigned char*)a, strlen(a));

		char str5[MAX_MD5_LEN];
		memset(str5, 0, MAX_MD5_LEN);

		u32 version = 0;
		u32 len = 0;
		u32 c_id = 0;
		u8  c_type = 0;
		read(c->ID, c_id);        //�ͻ���ID
		read(c->ID, c_type);      //�ͻ�������

		read(c->ID, version);       //�汾��
		read(c->ID, str5, MAX_MD5_LEN);//MD5��

		if (version != __ServerInfo->Version)
		{
			begin(c->ID, CMD_SECURITY);
			sss(c->ID, (u16)1);
			end(c->ID);
			return;
		}
		int error = stricmp(c->md5, str5);
		if (error != 0)
		{
			begin(c->ID, CMD_SECURITY);
			sss(c->ID, (u16)2);
			end(c->ID);
			return;
		}

		//��ȫ����
		c->state = S_SOCKET_STATE::S_CONNECT_SECURE;
		begin(c->ID, CMD_SECURITY);
		sss(c->ID, (u16)0);
		sss(c->ID, c->ID);
		end(c->ID);

		c->clientID = c_id;
		c->clientType = c_type;

		//ͨ���ͻ���ID �󶨹�ϵ 
		if (c->clientType != func::S_TYPE_USER) insertClientConnection(c->clientID, c);
		//����
		this->updateRecurityConnect(true);
		if (onSecureEvent != nullptr) this->onSecureEvent(this, c, 0);

		//int aa = m_ConnectCount;
		//int bb = m_SecurityCount;
		//LOG_MESSAGE("security connet...%d [%s:%d][connect:%d-%d]\n", (int)c->socketfd, c->ip, c->port, aa, bb);

		break;
	}
}
//�������
void net::TCPServer::checkConnect(S_CLIENT_BASE* c)
{
	s32 temp = 0;
	//0����鰲ȫ�ر�
	if (c->closeState == SOCKET_CLOSE::S_CLOSE_SHUTDOWN)
	{
		temp = (s32)time(NULL) - c->time_Close;
		if (c->is_RecvCompleted && c->is_SendCompleted)
		{
			closeSocket(c->socketfd, c, 2001);
		}
		else if (temp > 5)
		{
			//LOG_MESSAGE("��ȫ�ر�5��...%d %d \n", c->is_RecvCompleted, c->is_SendCompleted);
			closeSocket(c->socketfd, c, 2002);
		}
		return;
	}
	//1���������
	temp = (s32)time(NULL) - c->time_Connet;
	if (c->state == S_SOCKET_STATE::S_CONNECT)
	{
		if (temp > 10)
		{
			if (this->onTimeOutEvent != nullptr) onTimeOutEvent(this, c, 2002);
			shutDown(c->socketfd, 0, c, 2002);
			return;
		}
	}

	//2���������30��
	temp = (s32)time(NULL) - c->time_Heart;
	if (temp > __ServerInfo->HeartTime)
	{
		if (this->onTimeOutEvent != nullptr) onTimeOutEvent(this, c, 2003);
		shutDown(c->socketfd, 0, c, 2003);
		return;
	}
}
//*************************************************************************
//*************************************************************************
//*************************************************************************

S_CLIENT_BASE* TCPServer::client(const int id, const u32 clientid)
{
	if (clientid >= MAX_SERVER_ID)  return nullptr;

	if (id == MAX_SERVER_ID)
	{
		// ����CenterServer�л�ȡGameServer
		S_CLIENT_BASE* c = m_Connection[clientid];
		return c;
	}

	if (id < 0 || id >= Linkers->length) return nullptr;

	S_CLIENT_BASE * c = Linkers->Value(id);
	if (c->state >= S_SOCKET_STATE::S_CONNECT_SECURE &&
		c->clientID > 0 &&
		c->clientID == clientid)
	{
		return c;
	}

	c = m_Connection[clientid];
	return c;
}



S_CLIENT_BASE* TCPServer::client(const int id)
{
	if (id < 0 || id >= Linkers->length) return nullptr;

	S_CLIENT_BASE * c = Linkers->Value(id);
	return c;
}

S_CLIENT_BASE* net::TCPServer::client(SOCKET socketfd, bool isseriuty)
{
	if (socketfd < 0 || socketfd >= MAX_USER_SOCKETFD) return nullptr;
	S_CLIENT_BASE_INDEX * cindex = LinkersIndexs->Value(socketfd);
	if (cindex == nullptr) return nullptr;
	if (cindex->index < 0) return nullptr;

	S_CLIENT_BASE * c = client(cindex->index);
	if (c == nullptr)
	{
		int fd = socketfd;
		printf("Client c == null %d-%d line:%d\n", fd, cindex->index, __LINE__);
		return nullptr;
	}
	if (isseriuty)
	{
		if (!c->isT(socketfd)) return nullptr;
	}

	return c;
}

//�Զ���Ľṹ��,����TCP������
typedef struct tcp_keepalive
{
	unsigned long onoff;
	unsigned long keepalivetime;
	unsigned long keepaliveinterval;
}TCP_KEEPALIVE, * PTCP_KEEPALIVE;

//���ڼ��ͻȻ����,ֻ������windows 2000��ƽ̨
//���ͻ���Ҳ��Ҫwin2000����ƽ̨
int TCPServer::setHeartCheck(SOCKET s)
{
	DWORD dwError = 0L, dwBytes = 0;
	TCP_KEEPALIVE sKA_Settings = { 0 }, sReturned = { 0 };
	sKA_Settings.onoff = 1;
	sKA_Settings.keepalivetime = 5500; // Keep Alive in 5.5 sec.
	sKA_Settings.keepaliveinterval = 1000; // Resend if No-Reply

	dwError = WSAIoctl(s,
		SIO_KEEPALIVE_VALS,
		&sKA_Settings, sizeof(sKA_Settings),
		&sReturned, sizeof(sReturned),
		&dwBytes,
		NULL,
		NULL);
	if (dwError == SOCKET_ERROR)
	{
		dwError = WSAGetLastError();
		LOG_MESSAGE("SetHeartCheck->WSAIoctl()��������,�������: %ld  \n", dwError);
		return -1;
	}
	return 0;
}

S_CLIENT_BASE* net::TCPServer::getFreeLinker()
{
	std::lock_guard<std::mutex> guard(this->m_findlink_mutex);
	
	for (int i = 0; i < Linkers->length; i++)
	{
		S_CLIENT_BASE* client = Linkers->Value(i);
		if (client->state == S_SOCKET_STATE::S_FREE)
		{
			client->Reset();
			client->ID = i;
			client->state = S_SOCKET_STATE::S_CONNECT;
			return client;
		}
	}
	return nullptr;
}


bool  TCPServer::isID_T(const s32 id)
{
	if (id < 0 || id >= Linkers->length) return false;
	return true;
}
bool  TCPServer::isSecure_T(const s32 id, s32 secure)
{
	if (id < 0 || id >= Linkers->length) return false;
	S_CLIENT_BASE * c = Linkers->Value(id);
	if (c->state < secure) return false;
	return true;
}
bool  TCPServer::isSecure_F_Close(const s32 id, s32 secure)
{
	if (id < 0 || id >= Linkers->length) return false;
	S_CLIENT_BASE * c = Linkers->Value(id);
	if (c->state >= secure) return false;
	shutDown(c->socketfd, 0, c, 2006);
	return true;
}

//���Է������ݰ� ��װ
//#pragma pack(push,packing)
//#pragma pack(1)
//struct S_TEST
//{
//	u8  a;
//	u16 b;
//	s32 c;
//	char s[10];
//};
//#pragma pack(pop, packing)
//void testClientSendData(TCPServer*  tcp,net::S_CLIENT_BASE* c)
//{
//	S_TEST* test = new S_TEST();
//	test->a = 1;
//	test->b = 2l;
//	test->c = 3;
//	memset(&test->c, 0, 10);
//
//	tcp->begin(c->ID, (u16)1000);
//	tcp->sss(c->ID, (u8)9);
//	tcp->sss(c->ID, (u16)120);
//	tcp->sss(c->ID, test, sizeof(S_TEST));
//	tcp->end(c->ID);
//}
//*****************************************************************
//*****************************************************************
//*****************************************************************
//�������ݰ���װ��ʽ
void net::TCPServer::begin(const int id, const u16 cmd)
{
	auto c = client(id);
	if (c == nullptr) return;

	//ͷβ���
	if (c->send_Head == c->send_Tail)
	{
		c->send_Tail = 0;
		c->send_Head = 0;
	}
	c->send_TempTail = c->send_Tail;

	if (c->state > 0 &&
		c->is_Sending == false &&
		c->socketfd != INVALID_SOCKET &&
		c->send_TempTail + 8 <= __ServerInfo->SendMax)
	{
		c->is_Sending = true;
		
		c->sendBuf[c->send_Tail + 0] = __ServerInfo->Head[0] ^ c->rCode;
		c->sendBuf[c->send_Tail + 1] = __ServerInfo->Head[1] ^ c->rCode;

		u16 newcmd = cmd ^ c->rCode;
		char* a = (char*)& newcmd;
		c->sendBuf[c->send_Tail + 6] = a[0];
		c->sendBuf[c->send_Tail + 7] = a[1];

		c->send_TempTail += 8;
		return;
	}

	shutDown(c->socketfd, 0, c, 2004);
}

void net::TCPServer::end(const int id)
{
	auto c = client(id);
	if (c == nullptr) return;

	if (c->is_Sending == false ||
		c->send_Tail + 8 > __ServerInfo->SendMax ||
		c->send_TempTail > __ServerInfo->SendMax ||
		c->send_Tail >= c->send_TempTail)
	{
		shutDown(c->socketfd, 0, c, 2005);
		return;
	}
	c->is_Sending = false;
	u32 len = (c->send_TempTail - c->send_Tail) ^ c->rCode;
	char* a = (char*)& len;
	c->sendBuf[c->send_Tail + 2] = a[0];
	c->sendBuf[c->send_Tail + 3] = a[1];
	c->sendBuf[c->send_Tail + 4] = a[2];
	c->sendBuf[c->send_Tail + 5] = a[3];
	//��������ֵ
	c->send_Tail = c->send_TempTail;
}

void net::TCPServer::sss(const int id, const s8 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 1 <= __ServerInfo->SendMax)
	{
		
		c->sendBuf[c->send_TempTail] = v;
		c->send_TempTail++;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const u8 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 1 <= __ServerInfo->SendMax)
	{
		c->sendBuf[c->send_TempTail] = v;
		c->send_TempTail++;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const s16 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 2 <= __ServerInfo->SendMax)
	{
		char* p = (char*)& v;
		for (int i = 0; i < 2; i++)
			c->sendBuf[c->send_TempTail + i] = p[i];
		c->send_TempTail += 2;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const u16 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 2 <= __ServerInfo->SendMax)
	{
		char* p = (char*)& v;
		for (int i = 0; i < 2; i++)
			c->sendBuf[c->send_TempTail + i] = p[i];
		c->send_TempTail += 2;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const s32 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 4 <= __ServerInfo->SendMax)
	{
		char* p = (char*)& v;
		for (int i = 0; i < 4; i++)
			c->sendBuf[c->send_TempTail + i] = p[i];
		c->send_TempTail += 4;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const u32 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 4 <= __ServerInfo->SendMax)
	{
		char* p = (char*)& v;
		for (int i = 0; i < 4; i++)
			c->sendBuf[c->send_TempTail + i] = p[i];
		c->send_TempTail += 4;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const s64 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 8 <= __ServerInfo->SendMax)
	{
		char* p = (char*)& v;
		for (int i = 0; i < 8; i++)
			c->sendBuf[c->send_TempTail + i] = p[i];
		c->send_TempTail += 8;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const u64 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 8 <= __ServerInfo->SendMax)
	{
		char* p = (char*)& v;
		for (int i = 0; i < 8; i++)
			c->sendBuf[c->send_TempTail + i] = p[i];
		c->send_TempTail += 8;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const bool v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 8 <= __ServerInfo->SendMax)
	{
		c->sendBuf[c->send_TempTail] = v;
		c->send_TempTail += 1;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const f32 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 4 <= __ServerInfo->SendMax)
	{
		char* p = (char*)& v;
		for (int i = 0; i < 4; i++)
			c->sendBuf[c->send_TempTail + i] = p[i];
		c->send_TempTail += 4;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, const f64 v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + 8 <= __ServerInfo->SendMax)
	{
		char* p = (char*)& v;
		for (int i = 0; i < 8; i++)
			c->sendBuf[c->send_TempTail + i] = p[i];
		c->send_TempTail += 8;
		return;
	}

	c->is_Sending = false;
}

void net::TCPServer::sss(const int id, void* v, const u32 len)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + len <= __ServerInfo->SendMax)
	{
		memcpy(&c->sendBuf[c->send_TempTail], v, len);
		c->send_TempTail += len;
		return;
	}

	c->is_Sending = false;
}


//*********************************************************************
//*********************************************************************
//*********************************************************************
//��֤�ͻ�����Ч��
bool isValidClient(S_CLIENT_BASE* c, s32 value)
{
	if (c->ID == -1 ||
		c->state == func::S_FREE ||
		c->recv_TempTail == 0 ||
		c->recvBuf == nullptr ||
		c->recv_TempHead + value > c->recv_TempTail)
	{
		return false;
	}
	return true;
}

void net::TCPServer::read(const int id, s8& v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 1) == false)
	{
		v = 0;
		return;
	}
	v = (*(s8*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead++;
}

void net::TCPServer::read(const int id, u8 & v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 1) == false)
	{
		v = 0;
		return;
	}
	v = (*(u8*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead++;
}

void net::TCPServer::read(const int id, s16 & v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 2) == false)
	{
		v = 0;
		return;
	}
	v = (*(s16*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead += 2;
}

void net::TCPServer::read(const int id, u16 & v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 2) == false)
	{
		v = 0;
		return;
	}
	v = (*(u16*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead += 2;
}

void net::TCPServer::read(const int id, s32 & v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 4) == false)
	{
		v = 0;
		return;
	}
	v = (*(s32*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead += 4;
}

void net::TCPServer::read(const int id, u32 & v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 4) == false)
	{
		v = 0;
		return;
	}
	v = (*(u32*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead += 4;
}

void net::TCPServer::read(const int id, s64 & v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 8) == false)
	{
		v = 0;
		return;
	}
	v = (*(s64*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead += 8;
}

void net::TCPServer::read(const int id, u64 & v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 8) == false)
	{
		v = 0;
		return;
	}
	v = (*(u64*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead += 8;
}

void net::TCPServer::read(const int id, bool& v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 1) == false)
	{
		v = false;
		return;
	}
	v = (*(bool*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead += 1;
}

void net::TCPServer::read(const int id, f32 & v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 4) == false)
	{
		v = 0;
		return;
	}
	v = (*(f32*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead += 4;
}

void net::TCPServer::read(const int id, f64 & v)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, 8) == false)
	{
		v = 0;
		return;
	}
	v = (*(f64*)(c->recvBuf + c->recv_TempHead));
	c->recv_TempHead += 8;
}

void net::TCPServer::read(const int id, void* v, const u32 len)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, len) == false)
	{
		v = 0;
		return;
	}
	memcpy(v, &c->recvBuf[c->recv_TempHead], len);
	c->recv_TempHead += len;
}

void net::TCPServer::setOnClientAccept(TCPSERVERNOTIFY_EVENT event)
{
	onAcceptEvent = event;
}

void net::TCPServer::setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event)
{
	onSecureEvent = event;
}

void net::TCPServer::setOnClientDisConnect(TCPSERVERNOTIFY_EVENT event)
{
	onDisconnectEvent = event;
}

void net::TCPServer::setOnClientTimeOut(TCPSERVERNOTIFY_EVENT event)
{
	onTimeOutEvent = event;
}

void net::TCPServer::setOnClientExcept(TCPSERVERNOTIFY_EVENT event)
{
	onTimeOutEvent = event;
}

#endif