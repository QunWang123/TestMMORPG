#include "AppLogin.h"
#include "UserManager.h"
#include "DBBuffer.h"
#include "DBManager.h"
#include "ShareFunction.h"
#include "AppGlobal.h"
#include "AppGate.h"
#include "IDefine_Http.h"
#include <json.hpp>

using Json = nlohmann::json;

namespace app
{
	int temp_HttpthreadIndex = 0;
	AppLogin* __AppLogin = nullptr;
	void onLogin_Account(net::ITCPServer* ts, net::S_CLIENT_BASE* c);
	void onRegister_Account(net::ITCPServer* ts, net::S_CLIENT_BASE* c);


	AppLogin::AppLogin()
	{

	}

	AppLogin::~AppLogin()
	{

	}

	//登录成功
	void loginSuccess(int connectid, void* _login)
	{
		S_LOGIN_ACCOUNT* login = (S_LOGIN_ACCOUNT*)_login;

		//生成key MD5
		const int index = USER_MAX_KEY - 1;
		login->key[index] = '\0';
		std::string md5 = share::createMD5();
		memcpy(login->key, md5.c_str(), index);

		if (__GateList.size() == 0)
		{
			sendErrInfo(connectid, CMD_10, 1002);
			return;
		}

		//1、给中心服务器 发送KEY秘钥数据
		if (__TcpCenter->getData()->state >= func::S_CONNECT_SECURE)
		{
			__TcpCenter->begin(CMD_10);
			__TcpCenter->sss(login->name, USER_MAX_MEMBER);
			__TcpCenter->sss(login->key, USER_MAX_KEY);
			__TcpCenter->end();
		}

		// g是排序出来的最小负载的服务器
		auto g = __GateList[0];
		if (g == NULL) return;

		// 获取各条线玩家数量（把这些发给客户端，客户端会选择地图符合的最小压力的线路进入game服务器）
		std::vector<S_GAME_LINE*> arr;
		for (int i = 0; i < MAX_LINE_COUNT; i++)
		{
			if (__GameLine[i].isuse == 0) continue;
			arr.push_back(&__GameLine[i]);
		}

		//2、返回客户端登录结果
		LOG_MESSAGE("send gateip to client...\n");
		__TcpServer->begin(connectid, CMD_10);
		__TcpServer->sss(connectid, (u16)0);
		__TcpServer->sss(connectid, login->key, USER_MAX_KEY);

		//发送负担最小的网关IP PORT
		__TcpServer->sss(connectid, g->ip, MAX_IP_LEN);
		__TcpServer->sss(connectid, g->port);

		//发送各条线人数
// 		u8 count = arr.size();
// 		__TcpServer->sss(connectid, count);
// 		for (u8 i = 0; i < count; i++)
// 		{
// 			auto d = arr[i];
// 			__TcpServer->sss(connectid, d->line);
// 			__TcpServer->sss(connectid, d->count);
// 		}
 		__TcpServer->end(connectid);
	}

	void AppLogin::onInit()
	{

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
		case CMD_10:
			onLogin_Account(ts, c);
			break;
		case CMD_20:
			onRegister_Account(ts, c);
			break;
		default:
			break;
		}
		return true;
	}

	void onLogin_Account(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_LOGIN_ACCOUNT login;
		login.reset();
		login.connectid = c->ID;
		login.clientid = c->clientID;

		ts->read(c->ID, login.name, USER_MAX_MEMBER);
		ts->read(c->ID, login.password, USER_MAX_PASS);

#ifdef DEBUG_HTTP
		//推送到 http线程
		Json js;
		js["cmd"] = CMD_10;
		js["id"] = c->ID;
		js["socketid"] = c->socketfd;
		js["account"] = login.name;
		js["password"] = login.password;

		std::string str = js.dump();


		if (temp_HttpthreadIndex >= MAX_HTTP_THREAD) temp_HttpthreadIndex = 0;
		auto http = __HttpClient[temp_HttpthreadIndex];
		http->pushRequest("POST", "/checksdk", 3, str.c_str(), str.length());
		http->m_Condition.notify_one();
		temp_HttpthreadIndex++;
#else
		//登录成功
		loginSuccess(c->ID, &login);
#endif
	}

	void onRegister_Account(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_LOGIN_ACCOUNT  reg;
		reg.reset();
		reg.connectid = c->ID;
		reg.clientid = c->clientID;

		ts->read(c->ID, reg.name, USER_MAX_MEMBER);
		ts->read(c->ID, reg.password, USER_MAX_PASS);


#ifdef DEBUG_HTTP
		//推送到 http线程
		Json js;
		js["cmd"] = CMD_20;
		js["id"] = c->ID;
		js["socketid"] = c->socketfd;
		js["account"] = reg.name;
		js["password"] = reg.password;

		std::string str = js.dump();

		if (temp_HttpthreadIndex >= MAX_HTTP_THREAD) temp_HttpthreadIndex = 0;
		auto http = __HttpClient[temp_HttpthreadIndex];
		http->pushRequest("POST", "/checksdk", 3, str.c_str(), str.length());
		http->m_Condition.notify_one();
		temp_HttpthreadIndex++;
#endif
	}

	void onCMD_10(DBBuffer* buff)
	{
		u16 childcmd = 0;
		app::S_LOGIN_ACCOUNT login;

		buff->r(childcmd);
		buff->r(&login, sizeof(app::S_LOGIN_ACCOUNT));

		// 1. 插入账号
		if (childcmd == 0)
		{
			app::S_ACCOUNT_BASE* acc = __UserManager->findAccount(login.name);
			if (acc == NULL)
			{
				acc = new app::S_ACCOUNT_BASE();
				acc->reset();
				memcpy(acc->name, login.name, USER_MAX_MEMBER);
				memcpy(acc->password, login.password, USER_MAX_PASS);
				__UserManager->insetAccount(acc);
			}
		}

		// 2. 查找连接
		auto c = __TcpServer->client(login.connectid);
		if (c == nullptr || c->clientID != login.clientid)
		{
			LOG_MESSAGE("onCMD_10err ... %d-%d", login.connectid, login.clientid);
			return;
		}

		if (childcmd != 0)
		{
			sendErrInfo(c->ID, CMD_10, childcmd);
			return;
		}

		// 登录成功
		loginSuccess(c->ID, &login);
	}

	// 收到DB返回注册信息
	void onCMD_20(DBBuffer* buff)
	{
		u16 childcmd = 0;
		app::S_LOGIN_ACCOUNT reg;

		buff->r(childcmd);
		buff->r(&reg, sizeof(app::S_LOGIN_ACCOUNT));

		if (childcmd == 0)
		{
			app::S_ACCOUNT_BASE* acc = new app::S_ACCOUNT_BASE();
			acc->reset();
			memcpy(acc->name, reg.name, USER_MAX_MEMBER);
			memcpy(acc->password, reg.password, USER_MAX_PASS);
			__UserManager->insetAccount(acc);
		}

		auto c = __TcpServer->client(reg.connectid);
		if (c == nullptr || c->clientID != reg.clientid)
		{
			LOG_MESSAGE("onCMD_10err ... %d-%d", reg.connectid, reg.clientid);
			return;
		}

		sendErrInfo(c->ID, CMD_20, childcmd);
	}

	// 收到数据库返回指令数据
	bool AppLogin::onDBCommand(void* _buff, const u16 cmd)
	{
		DBBuffer* buff = (DBBuffer*)_buff;
		if (buff == nullptr) return false;

		switch (cmd)
		{
		case CMD_10:
			onCMD_10(buff);
			break;
		case CMD_20:
			onCMD_20(buff);
			break;
		}
	}
}


