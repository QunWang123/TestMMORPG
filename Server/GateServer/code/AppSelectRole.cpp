#include "AppSelectRole.h"
#include "UserManager.h"
#include "WorldData.h"
#include "AppGlobal.h"

namespace app
{
	AppSelectRole* __AppSelectRole = nullptr;
	
	AppSelectRole::AppSelectRole()
	{

	}

	AppSelectRole::~AppSelectRole()
	{

	}

	void AppSelectRole::onInit()
	{

	}

	void onSelectRole(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_SELECT_ROLE role;
		ts->read(c->ID, role.roleindex);
		ts->read(c->ID, role.memid);
		role.user_connectid = c->ID;

		// 1.验证玩家基础数据
		auto user = __UserManager->findUser(c->ID, role.memid);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_200, 4001);
			return;
		}

		// 2.时间验证，2秒内只能有一条指令
		int ftime = (int)time(NULL) - user->temp_RoleTime;
		if (ftime < 2)
		{
			sendErrInfo(ts, c->ID, CMD_200, 4002);
			return;
		}
		user->temp_RoleTime = (int)time(NULL);
		// 角色的位置不能越界
		if (role.roleindex >= USER_MAX_ROLE)
		{
			sendErrInfo(ts, c->ID, CMD_200, 4003);
			return;
		}
		// 验证连接中心服务器了吗
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_200, 4004);
			return;
		}

		__TcpCenter->begin(CMD_200);
		__TcpCenter->sss(&role, sizeof(S_SELECT_ROLE));
		__TcpCenter->end();
	}

	void onCreateRole(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_CREATE_ROLE role;
		ts->read(c->ID, role.memid);				// 传memid主要是确保用户没问题,不能随便就创建角色
		ts->read(c->ID, role.job);
		ts->read(c->ID, role.sex);
		ts->read(c->ID, role.nick, USER_MAX_NICK);
		role.user_connectid = c->ID;

		// 1.验证玩家基础数据
		auto user = __UserManager->findUser(c->ID, role.memid);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_300, 4001);
			return;
		}

		// 2.时间验证，2秒内只能有一条指令
		int ftime = (int)time(NULL) - user->temp_RoleTime;
		if (ftime < 2)
		{
			sendErrInfo(ts, c->ID, CMD_300, 4002);
			return;
		}
		user->temp_RoleTime = (int)time(NULL);

		// 验证连接中心服务器了吗
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_300, 4003);
			return;
		}

		__TcpCenter->begin(CMD_300);
		__TcpCenter->sss(&role, sizeof(S_CREATE_ROLE));
		__TcpCenter->end();

	}

	void onDeleteRole(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_DELETE_ROLE role;
		ts->read(c->ID, role.memid);				// 传memid主要是确保用户没问题,不能随便就创建角色
		ts->read(c->ID, role.roleindex);
		ts->read(c->ID, role.roleid);
		role.user_connectid = c->ID;

		// 1.验证玩家基础数据
		auto user = __UserManager->findUser(c->ID, role.memid);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_400, 4001);
			return;
		}

		// 2.时间验证，2秒内只能有一条指令
		int ftime = (int)time(NULL) - user->temp_RoleTime;
		if (ftime < 2)
		{
			sendErrInfo(ts, c->ID, CMD_400, 4002);
			return;
		}
		user->temp_RoleTime = (int)time(NULL);

		// 验证连接中心服务器了吗
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_400, 4003);
			return;
		}

		__TcpCenter->begin(CMD_400);
		__TcpCenter->sss(&role, sizeof(S_DELETE_ROLE));
		__TcpCenter->end();
	}

	bool AppSelectRole::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppSelectRole err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_USER) return false;

		switch (cmd)
		{
		case CMD_200:
			onSelectRole(ts, c);
			break;
		case CMD_300:
			onCreateRole(ts, c);
			break;
		case CMD_400:
			onDeleteRole(ts, c);
			break;
		}
		return true;
	}

	/*******************************************************************************************/
	void onCMD_200(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_SELECT_ROLE role;
		tc->read(childcmd);
		tc->read(&role, sizeof(S_SELECT_ROLE));

		S_USER_GATE* user = __UserManager->findUser_Connection(role.user_connectid, role.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSelect onCMD_200 Err ... user == null [%d-%lld] line:%d \n",
				role.user_connectid, role.memid, __LINE__);
			return;
		}

		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, user->connection->ID, CMD_200, childcmd);
			return;
		}

		user->memid = role.memid;
		user->mapid = role.mapid;
		// role.userindex是用户在DB服务器上的索引
		user->userindex = role.userindex;
		user->tcpGame = __UserManager->findGameServer_Connection(user->line, user->mapid);		// 通过地图ID和线路获得连接哪个游戏服务器
	
		if (user->tcpGame == NULL)
		{
			LOG_MESSAGE("CMD_200 target GameServer = NULL, line:%d\n", __LINE__);
			sendErrInfo(__TcpServer, user->connection->ID, CMD_200, 4010);
			return;
		}

		if (user->tcpGame->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, user->connection->ID, CMD_200, 4011);
			return;
		}

		// 发送数据给游戏服务器
		S_LOAD_ROLE data;
		data.user_connectid = role.user_connectid;
		data.memid = role.memid;
		data.mapid = role.mapid;
		data.userindex = role.userindex;

		user->tcpGame->begin(CMD_500);
		user->tcpGame->sss(&data, sizeof(S_LOAD_ROLE));
		user->tcpGame->end();
	}

	void onCMD_300(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_CREATE_ROLE role;

		tc->read(childcmd);
		tc->read(&role, sizeof(S_CREATE_ROLE));

		auto user = __UserManager->findUser_Connection(role.user_connectid, role.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSelect Err ... user == null [%d-%lld] line:%d \n",
				role.user_connectid, role.memid, __LINE__);
			return;
		}

		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, role.user_connectid, CMD_300, childcmd);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_300);
		__TcpServer->sss(c->ID, childcmd);
		__TcpServer->sss(c->ID, role.roleindex);
		__TcpServer->sss(c->ID, role.roleid);
		__TcpServer->sss(c->ID, role.job);
		__TcpServer->sss(c->ID, role.sex);
		__TcpServer->sss(c->ID, role.nick, USER_MAX_NICK);
		__TcpServer->end(c->ID);
	}

	void onCMD_400(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_CREATE_ROLE role;

		tc->read(childcmd);
		tc->read(&role, sizeof(S_DELETE_ROLE));

		auto user = __UserManager->findUser_Connection(role.user_connectid, role.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSelect Err ... user == null [%d-%lld] line:%d \n",
				role.user_connectid, role.memid, __LINE__);
			return;
		}

		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, role.user_connectid, CMD_400, childcmd);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_400);
		__TcpServer->sss(c->ID, childcmd);
		__TcpServer->sss(c->ID, role.roleindex);
		__TcpServer->end(c->ID);
	}

	void onCMD_500(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_LOAD_ROLE data;

		tc->read(childcmd);
		tc->read(&data, sizeof(S_LOAD_ROLE));

		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSelect Err ... user == null [%d-%lld] line:%d \n",
				data.user_connectid, data.memid, __LINE__);
			return;
		}

		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_500, childcmd);
			return;
		}

		// 解析玩家基础数据
		S_ROLE_BASE rolebase;
		tc->read(&rolebase, sizeof(S_ROLE_BASE));
		// 设置
		user->state = M_LOGINEND;
		c->state = func::S_LOGIN;
		user->level = rolebase.exp.level;
		memcpy(user->nick, rolebase.innate.nick, USER_MAX_NICK);

		// 这里这个实际上是发给聊天、组队服务器的
		__UserManager->sendWorld_UserEntryWorld(user);

		LOG_MESSAGE("AppSelect CMD_500 Success... [%d-%lld] nick:%s line:%d \n",
			data.user_connectid, data.memid, user->nick, __LINE__);

		// 解析玩家技能数据
		S_ROLE_STAND_SKILL role_stand_skill;
		tc->read(&role_stand_skill, sizeof(S_ROLE_STAND_SKILL));

		__TcpServer->begin(c->ID, CMD_500);
		__TcpServer->sss(c->ID, childcmd);
		__TcpServer->sss(c->ID, &rolebase.exp, sizeof(S_ROLE_BASE_EXP));
		__TcpServer->sss(c->ID, &rolebase.econ, sizeof(S_ROLE_BASE_ECON));
		__TcpServer->sss(c->ID, &rolebase.status, 19);
		__TcpServer->sss(c->ID, &rolebase.life, sizeof(S_ROLE_BASE_LIFE));
		__TcpServer->sss(c->ID, user->userindex);
		__TcpServer->sss(c->ID, &role_stand_skill, sizeof(S_ROLE_STAND_SKILL));

		__TcpServer->end(c->ID);
	}

	bool AppSelectRole::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (cmd < CMD_500)
		{
			if (tc->getData()->serverType != func::S_TYPE_CENTER) return false;
		}
		else
		{
			if (tc->getData()->serverType != func::S_TYPE_GAME) return false;
		}

		switch (cmd)
		{
		case CMD_200:
			onCMD_200(tc);
			break;
		case CMD_300:
			onCMD_300(tc);
			break;
		case CMD_400:
			onCMD_400(tc);
			break;
		case CMD_500:
			onCMD_500(tc);
			break;
		}
		return true;
	}
}



