#include "AppLogin.h"
#include "UserManager.h"
#include "ShareFunction.h"
#include "AppGlobal.h"
#include "CommandData2.h"
#include "WorldBC.h"
#include "AppTeam.h"


namespace app
{
	AppLogin* __AppLogin = nullptr;

	void sendErrInfo(s32 connectID, const u16 cmd, const u16 childcmd, S_LOGIN_GAME* login)
	{
		__TcpServer->begin(connectID, cmd);
		__TcpServer->sss(connectID, childcmd);
		__TcpServer->sss(connectID, login, sizeof(S_LOGIN_GAME));
		__TcpServer->end(connectID);
	}

	AppLogin::AppLogin()
	{

	}

	AppLogin::~AppLogin()
	{

	}

	void AppLogin::onInit()
	{

	}

	//  �յ���¼���������͵���Կ��Ϣ
	void onSecurity_Key(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		char name[USER_MAX_MEMBER];		// �˺�
		char key[USER_MAX_KEY];			// ��Կ

		ts->read(c->ID, name, USER_MAX_MEMBER);
		ts->read(c->ID, key, USER_MAX_KEY);

		S_ACCOUNT_KEY* acc = __UserManager->findAccountKey(name);
		if (acc == NULL)
		{
			acc = new S_ACCOUNT_KEY();
			memcpy(acc->name, name, USER_MAX_MEMBER);
			__UserManager->insetAccount(acc);
		}
		memcpy(acc->key, key, USER_MAX_KEY);
		acc->keytime = (int)time(NULL);
		LOG_MESSAGE("set Security_Key %s-%s  %s-%s\n", acc->name, name, acc->key, key);
	}

	// ������ߵ�¼��Ϸ
	void onLoginGame(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		// login�е�userid�Ѿ����������úò��������ˣ�userid��ָ�����û��ģ�ServerConnectID��ָ�����صģ�

		S_LOGIN_GAME login;
		ts->read(c->ID, &login, sizeof(S_LOGIN_GAME));
		login.ServerConnectID = c->ID;
		login.ServerClientID = c->clientID;

		LOG_MESSAGE("account login...name:%s key:%s \n", login.name, login.key);
		//0����֤�������ݿ����������
		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(c->ID, CMD_100, 2001, &login);
			return;
		}
		//1���˺Ų�����
		auto acc = __UserManager->findAccountKey(login.name);
		if (acc == NULL)
		{
			LOG_MESSAGE("account not find...%s \n", login.name);
			sendErrInfo(c->ID, CMD_100, 2002, &login);
			return;
		}
		//2����֤��Կ
		bool iskey = acc->isT_Key(login.key);
		if (iskey == false)
		{
			sendErrInfo(c->ID, CMD_100, 2003, &login);
			return;
		}
		//3����֤��Чʱ��
		int value = time(NULL) - acc->keytime;
		if (value > 300)
		{
			sendErrInfo(c->ID, CMD_100, 2004, &login);
			return;
		}

		__TcpDB->begin(CMD_100);
		__TcpDB->sss(&login, sizeof(S_LOGIN_GAME));
		__TcpDB->end();
	}

	void onHeart(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex = 0;
		u64 memid;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			LOG_MESSAGE("onheart DB disconnect... %d-%lld\n",
				userindex, memid);
		}
		else
		{
			__TcpDB->begin(CMD_90);
			__TcpDB->sss(userindex);
			__TcpDB->sss(memid);
			__TcpDB->end();
		}

		auto user = __UserManager->findUser(userindex, memid);
		
		if (user == nullptr) return;

		user->tmp.temp_HeartTime = (int)time(NULL);
		if (user->mem.state == M_LOGINING) user->mem.state = M_LOGINEND;
	}

	// �յ����ʧȥ������Ϣ
	void onUserDisconnect(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 userindex = 0;
		u64 memid;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		if (__TcpDB->getData()->state >= func::C_CONNECT_SECURE)
		{
			__TcpDB->begin(CMD_600);
			__TcpDB->sss(userindex);
			__TcpDB->sss(memid);
			__TcpDB->end();
		}

		if (__UserManager == nullptr) return;
		auto user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("leaveWorld  not find user...  %d-%lld, %d\n",  
				userindex, memid, __LINE__);
			return;
		}
		
		LOG_MESSAGE("onUserDisconnect %d-%lld-%s, %d\n",
			userindex, memid, user->mem.name, __LINE__);
		__AppTeam->setTeamLeave(user, EDT_DISCONNET_USER);
		user->reset();

		__UserManager->__OnlineCount--;
	}

	// �յ��ͻ��˷���������ָ������
	bool AppLogin::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("APPLogin err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE && c->clientType != func::S_TYPE_LOGIN) return false;

		switch (cmd)
		{
		case CMD_10:
			onSecurity_Key(ts, c);
			break;
		case CMD_90:
			onHeart(ts, c);
			break;
		case CMD_100:
			onLoginGame(ts, c);
			break;
		case CMD_600:
			onUserDisconnect(ts, c);
			break;
		}
		return true;
	}


	// �յ���DB������������CMD_100
	void onCMD_100(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_LOGIN_GAME login;

		tc->read(childcmd);
		tc->read(&login, sizeof(S_LOGIN_GAME));

		// c�����ط�����
		auto c = __TcpServer->client(login.ServerConnectID, login.ServerClientID);
		if (c == nullptr)
		{
			LOG_MESSAGE("Applogin err c == NULL [%s-%d-%d] %d\n", login.name, login.ServerConnectID, login.ServerClientID, __LINE__);
			return;
		}

		if (childcmd != 0)
		{
			sendErrInfo(c->ID, CMD_100, childcmd, &login);
			return;
		}

		// ��ȡ��ɫ�˺�ID
		u64 memid = 0;
		tc->read(memid);

		// ���ս�ɫ��������
		S_USER_MEMBER_ROLE role[USER_MAX_ROLE];
		tc->read(&role, sizeof(S_USER_MEMBER_ROLE) * USER_MAX_ROLE);

		__TcpServer->begin(c->ID, CMD_100);
		__TcpServer->sss(c->ID, u16(0));
		__TcpServer->sss(c->ID, &login, sizeof(S_LOGIN_GAME));
		__TcpServer->sss(c->ID, memid);
		__TcpServer->sss(c->ID, role, sizeof(S_USER_MEMBER_ROLE) * USER_MAX_ROLE);
		__TcpServer->end(c->ID);
	}
	
	bool AppLogin::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_DB) return false;

		switch (cmd)
		{
		case CMD_100:
			onCMD_100(tc);
			break;
		}
		return false;
	}

}


