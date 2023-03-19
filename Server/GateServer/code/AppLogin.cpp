#include "AppLogin.h"
#include "UserManager.h"
#include "ShareFunction.h"
#include "AppGlobal.h"


namespace app
{
	AppLogin* __AppLogin = nullptr;
	
	AppLogin::AppLogin()
	{

	}

	AppLogin::~AppLogin()
	{

	}

	void AppLogin::onInit()
	{

	}

	// ��Ϊ���ط���������ֹҪȷ���Ϳͻ��˵�CMD_HEART ��Ҫ�������ϴ�CMD_90����
	// engine�е�CMD_HEARTֻ�����ͻ��˷��أ�û��CMD_90����
	// ������Ϊgateserver���յ��ͻ��˵�CMD_HEART֮���߹�engine, ��Ҫ��һ�����
	// �������ĺͿͻ��˷���CMD_90
	// ����Ļ��������ÿͻ��˵�����ʱ�䣬��Ϊengine���������ù���
	// ��CMD_90ֻ�������ж��û��Ƿ����ߵ�������CMD_HEART���������жϿͻ��˺ͷ����֮������ӵ�
	// CMD_90ֻ��Ҫ���ϴ���Ok,��DB�������������߾����˰ѣ�
	void onHeart(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_USER_GATE* user = __UserManager->findUser(c->ID, c->memid);
		// �û�Ҫ�ж�Ӧ������
		if (user == nullptr || user->memid == 0) return;

		if (__TcpCenter)
		{
			__TcpCenter->begin(CMD_90);
			__TcpCenter->sss(user->userindex);
			__TcpCenter->sss(user->memid);
			__TcpCenter->end();
		}
		
		if (user->tcpGame == nullptr) return;

		net::ITCPClient* tcp = user->tcpGame;
		tcp->begin(CMD_90);
		tcp->sss(user->userindex);
		tcp->sss(user->memid);
		tcp->end();
	}

	// ������ߵ�¼��Ϸ
	void onLoginGame(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_LOGIN_GAME login;

		ts->read(c->ID, login.name, USER_MAX_MEMBER);		// ��ȡ�˺�
		ts->read(c->ID, login.key, USER_MAX_KEY);			// ��ȡ��Կ
		ts->read(c->ID, login.line);						// ѡ�����·

		login.UserConnectID = c->ID;
		login.UserClientID = c->clientID;

		int ftime = (int)time(NULL) - c->temp_LoginTime;
		if (ftime < 1)
		{
			// ͬһ���û�����2���ܵ����ε�¼
			sendErrInfo(ts, login.UserConnectID, CMD_100, 4001);
			return;
		}

		if (strlen(login.name) < 3 || strlen(login.key) < 10)
		{
			sendErrInfo(ts, login.UserConnectID, CMD_100, 4002);
			return;
		}
		// ��֤�������ķ���������
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, login.UserConnectID, CMD_100, 4003);
			return;
		}

		c->temp_LoginTime = (int)time(NULL);
		// �����ķ�����������Ϣ
		__TcpCenter->begin(CMD_100);
		__TcpCenter->sss(&login, sizeof(S_LOGIN_GAME));
		__TcpCenter->end();
	}

	// �յ��ͻ��˷���������ָ������
	bool AppLogin::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("APPLogin err ... line:%d", __LINE__);
			return false;
		}

		switch (cmd)
		{
		case CMD_HEART:
			onHeart(ts, c);
			break;
		case CMD_100:
			onLoginGame(ts, c);
			break;
		}
		return true;
	}

	void onCMD_100(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_LOGIN_GAME login;

		tc->read(childcmd);
		tc->read(&login, sizeof(S_LOGIN_GAME));

		auto c = __TcpServer->client(login.UserConnectID);
		if (c == nullptr || c->clientID != login.UserClientID)
		{
			LOG_MESSAGE("AppLogin Err ... c == null [%s-%d-%d] %d\n",
				login.name, login.UserConnectID, login.UserClientID, __LINE__);
			return;
		}

		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_100, childcmd);
			return;
		}

		u64 memid = 0;
		tc->read(memid);

		app:S_USER_GATE* user = __UserManager->findUser(login.UserConnectID);
		if (user == NULL)
		{
			LOG_MESSAGE("AppLogin Err ... c == null [%s-%d-%d] %d\n",
				login.name, login.UserConnectID, login.UserClientID, __LINE__);
			return;
		}

#ifdef TEST_UDP_SERVER
		//UDP �����˺�
		auto udp = __IUdpServer->findClient(c->ID);
		if (udp != NULL) udp->memid = memid;
#endif

		user->reset();
		// ���ս�ɫ��������
		S_USER_MEMBER_ROLE role[USER_MAX_ROLE];
		tc->read(&role, sizeof(S_USER_MEMBER_ROLE) * USER_MAX_ROLE);

		//������һ�������
		user->line = login.line;
		user->memid = memid;
		user->connection = c;
		user->temp_RoleTime = 0;

		c->memid = memid;
		memcpy(user->name, login.name, USER_MAX_MEMBER);

		__TcpServer->begin(c->ID, CMD_100);
		__TcpServer->sss(c->ID, u16(0));
		__TcpServer->sss(c->ID, memid);
		__TcpServer->sss(c->ID, role, sizeof(S_USER_MEMBER_ROLE) * USER_MAX_ROLE);
		__TcpServer->end(c->ID);
		
	}
	// ���ط�������Ϊ�ͻ���ʱ�յ�����Ϣ
	bool AppLogin::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_CENTER) return false;

		switch (cmd)
		{
		case CMD_100:
			onCMD_100(tc);
			break;
		}
		return false;
	}
}


