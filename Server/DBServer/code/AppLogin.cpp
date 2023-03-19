#include "AppLogin.h"
#include "UserManager.h"
#include "DBBuffer.h"
#include "DBManager.h"
#include "ShareFunction.h"
#include "AppGlobal.h"


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

	void sendData_RoleBase(s32 connectid, u16 cmd, u16 childcmd, S_LOGIN_GAME* login, S_USER_MEMBER_BASE* m)
	{
		__TcpServer->begin(connectid, cmd);
		__TcpServer->sss(connectid, childcmd);
		__TcpServer->sss(connectid, login, sizeof(S_LOGIN_GAME));
		__TcpServer->sss(connectid, m->mem.id);			// 游戏账号id
		__TcpServer->sss(connectid, &m->role, sizeof(S_USER_MEMBER_BASE) * USER_MAX_ROLE);
		__TcpServer->end(connectid);
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

	void onHeart(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex = 0;
		u64 memid;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		auto m = __UserManager->findMember(memid);

		if (m != nullptr)
		{
			m->tmp.temp_HeartTime = (int)time(NULL);
		}
		if (m->mem.state < M_LOGINEND) return;

		auto user = __UserManager->findUser(userindex, memid);

		if (user == nullptr)
		{
			LOG_MESSAGE("heart error... %d-%lld\n",
				userindex, memid);
			return;
		}

		user->tmp.temp_HeartTime = (int)time(NULL);
	}

	void onLoginGame(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_LOGIN_GAME login;
		ts->read(c->ID, &login, sizeof(S_LOGIN_GAME));

		// 全部验证通过,但如果数据库这边没有账号的话就注册一个
		auto m = __UserManager->findMember(login.name);
		if (m == NULL)
		{
			// 可以注册一个
			auto db = __DBManager->GetDBSource(ETT_USERWRITE);
			auto buff = db->PopBuffer();
			buff->b(CMD_100);
			buff->s(c->ID);
			buff->s(c->clientID);
			buff->s(&login, sizeof(S_LOGIN_GAME));
			buff->e();
			db->PushToThread(buff);

			return;
		}

		//状态 不是空闲 状态
		if (m->mem.state != M_FREE)
		{
			u16 childcmd = 1000 + m->mem.state;
			sendErrInfo(c->ID, CMD_100, childcmd, &login);
			return;
		}

		//成功
		m->mem.state = M_SELECTROLE;
		m->tmp.temp_HeartTime = (int)time(NULL);
		__UserManager->insertMemberStates(m);
		//返回角色基础数据给中心服务器
		sendData_RoleBase(c->ID, CMD_100, 0, &login, m);
	}

	void onUserDisconnect(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 userindex = 0;
		u64 memid;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		auto m = __UserManager->findMember(memid);
		if (m == NULL)
		{
			LOG_MESSAGE("onUserDisconnect m == nullptr  %d-%lld, %d\n",
				userindex, memid, __LINE__);
			return;
		}

		if (c->clientType == func::S_TYPE_CENTER)
		{
			LOG_MESSAGE("onUserDisconnect center  %lld-%d, %d\n",
				memid, m->mem.state, __LINE__);
			// 中心服务器提示玩家掉线，但玩家的状态是M_SELECTROLE或者M_LOGINING
			// 说明玩家转到游戏服务器上了，不会走下面的正常掉线流程
			if (m->mem.state == M_SELECTROLE || m->mem.state == M_LOGINING)
			{
				m->mem.state = M_FREE;
			}
			return;
		}

		u32 timelogin = 0;
		ts->read(c->ID, timelogin);
		if (m->mem.timeLastLogin != timelogin)
		{
			LOG_MESSAGE("onUserDisconnect timelogin %d-%lld, time:%d-%d, line:%d\n",
				m->mem.state, memid, m->mem.timeLastLogin, timelogin, __LINE__);
			return;
		}
		if (m->mem.state != M_LOGINEND)
		{
			LOG_MESSAGE("onUserDisconnect state error  %d-%lld, line:%d\n",
				m->mem.state, memid, __LINE__);
			return;
		}

		auto user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onUserDisconnect user == nullptr  %lld, %d\n",
				memid, __LINE__);
			return;
		}

		m->mem.state = M_SAVING;
		user->mem.state = M_SAVING;

		LOG_MESSAGE("onUserDisconnect 600  %lld, line:%d\n",
			memid, __LINE__);
		// 200是正常掉线，100是心跳包掉线，0是自动保存
		__UserManager->userDisconnect(userindex, memid, 200);
		__UserManager->updateMemberMinrole(user);
	}

	// 保存新手玩家基本数据	
	void onUserInitNew(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex;
		u64 memid;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		auto m = __UserManager->findMember(memid);
		if (m == NULL)
		{
			LOG_MESSAGE("onUserInitNew m == nullptr  %d-%lld, %d\n",
				userindex, memid, __LINE__);
			return;
		}
		if (m->mem.state != M_LOGINEND)
		{
			LOG_MESSAGE("onUserInitNew state error  %d-%lld, line:%d\n",
				m->mem.state, memid, __LINE__);
			return;
		}
		auto user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onUserInitNew user == nullptr  %lld, %d\n",
				memid, __LINE__);
			return;
		}
		ts->read(c->ID, &user->role.base, sizeof(S_ROLE_BASE));
		ts->read(c->ID, &user->role.stand.myskill, sizeof(S_ROLE_STAND_SKILL));
		ts->read(c->ID, &user->role.stand.atk, sizeof(S_SPRITE_ATTACK));
		ts->read(c->ID, user->role.stand.bag.num);
		LOG_MESSAGE("onUserInitNew 610  %lld, %d\n",
			memid, __LINE__);
		__UserManager->userBaseData(userindex, memid);
	}

	// 收到客户端发送上来的指令数据
	bool AppLogin::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("APPLogin err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_CENTER && c->clientType != func::S_TYPE_GAME) return false;

		switch (cmd)
		{
		case CMD_90:
			onHeart(ts, c);
			break;
		case CMD_100:
			onLoginGame(ts, c);
			break;
		case CMD_600:
			onUserDisconnect(ts, c); 
			break;//玩家掉线
		case CMD_610:
			onUserInitNew(ts, c);
			break;
		}

		return true;
	}



	void onCMD_100(DBBuffer* buff)
	{
		u16 childcmd;
		s32 connectid;
		s32 clientid;
		s64 memid;
		s32 tableid;
		S_LOGIN_GAME login;

		buff->r(childcmd);
		buff->r(connectid);
		buff->r(clientid);
		buff->r(memid);
		buff->r(tableid);
		buff->r(&login, sizeof(S_LOGIN_GAME));

		// 先找到中心服务器
		auto c = __TcpServer->client(connectid, clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("CMD_100 error... %d-%lld line:%d\n", connectid, clientid, __LINE__);
			return;
		}
		// 自动注册账号失败
		if (childcmd != 0)
		{
			sendErrInfo(c->ID, CMD_100, childcmd, &login);
			return;
		}

		auto m = __UserManager->findMember(login.name);
		if (m != NULL)
		{
			sendErrInfo(c->ID, CMD_100, 1021, &login);
			return;
		}

		m = new S_USER_MEMBER_BASE();
		m->mem.id = memid;
		m->mem.areaID = func::__ServerInfo->ID;
		m->mem.tableID = tableid;
		memcpy(m->mem.name, login.name, USER_MAX_MEMBER);

		m->mem.state = M_SELECTROLE;
		m->tmp.temp_HeartTime = (int)time(NULL);
		__UserManager->insertMember(m);
		__UserManager->insertMemberStates(m);

		sendData_RoleBase(c->ID, CMD_100, 0, &login, m);
	}

	void onCMD_600(DBBuffer* buff)
	{
		u16 childcmd = 0;
		s32 userindex;
		s64 memid;
		buff->r(childcmd);
		buff->r(userindex);
		buff->r(memid);

		auto m = __UserManager->findMember(memid);
		if (m != NULL)
		{
			m->mem.state = M_FREE;
		}
		else
		{
			LOG_MESSAGE("CMD_600 error... %d-%lld line:%d\n", userindex, memid, __LINE__);
		}

		auto user = __UserManager->findUser(userindex, memid);
		if (user != NULL)
		{
			user->reset();
		}
		else
		{
			LOG_MESSAGE("CMD_600 error... %d-%lld line:%d\n", userindex, memid, __LINE__);
		}

	}

	// 收到数据库返回指令数据
	bool AppLogin::onDBCommand(void* _buff, const u16 cmd)
	{
		DBBuffer* buff = (DBBuffer*)_buff;
		if (buff == nullptr) return false;

		switch (cmd)
		{
		case CMD_100:
			onCMD_100(buff);
			break;
		case CMD_600:
			onCMD_600(buff);
			break;
		}
	}
}


