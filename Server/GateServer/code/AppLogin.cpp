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

	// 作为网关服务器，不止要确保和客户端的CMD_HEART 还要往中心上传CMD_90命令
	// engine中的CMD_HEART只有往客户端返回，没有CMD_90命令
	// 所以作为gateserver，收到客户端的CMD_HEART之后，走过engine, 还要走一下这里，
	// 即向中心和客户端发送CMD_90
	// 这里的话无需重置客户端的心跳时间，因为engine中重新设置过了
	// 即CMD_90只是用来判断用户是否在线的心跳，CMD_HEART才是用来判断客户端和服务端之间的连接的
	// CMD_90只需要往上传就Ok,在DB服务器更新在线就行了把？
	void onHeart(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_USER_GATE* user = __UserManager->findUser(c->ID, c->memid);
		// 用户要有对应的连接
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

	// 进入或者登录游戏
	void onLoginGame(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_LOGIN_GAME login;

		ts->read(c->ID, login.name, USER_MAX_MEMBER);		// 读取账号
		ts->read(c->ID, login.key, USER_MAX_KEY);			// 读取密钥
		ts->read(c->ID, login.line);						// 选择的线路

		login.UserConnectID = c->ID;
		login.UserClientID = c->clientID;

		int ftime = (int)time(NULL) - c->temp_LoginTime;
		if (ftime < 1)
		{
			// 同一个用户不能2秒能点两次登录
			sendErrInfo(ts, login.UserConnectID, CMD_100, 4001);
			return;
		}

		if (strlen(login.name) < 3 || strlen(login.key) < 10)
		{
			sendErrInfo(ts, login.UserConnectID, CMD_100, 4002);
			return;
		}
		// 验证连接中心服务器了吗
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, login.UserConnectID, CMD_100, 4003);
			return;
		}

		c->temp_LoginTime = (int)time(NULL);
		// 向中心服务器发送信息
		__TcpCenter->begin(CMD_100);
		__TcpCenter->sss(&login, sizeof(S_LOGIN_GAME));
		__TcpCenter->end();
	}

	// 收到客户端发送上来的指令数据
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
		//UDP 设置账号
		auto udp = __IUdpServer->findClient(c->ID);
		if (udp != NULL) udp->memid = memid;
#endif

		user->reset();
		// 接收角色基础数据
		S_USER_MEMBER_ROLE role[USER_MAX_ROLE];
		tc->read(&role, sizeof(S_USER_MEMBER_ROLE) * USER_MAX_ROLE);

		//设置玩家基础数据
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
	// 网关服务器作为客户端时收到的信息
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


