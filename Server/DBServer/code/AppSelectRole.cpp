#include "AppSelectRole.h"
#include "UserManager.h"
#include "WorldData.h"
#include "AppGlobal.h"
#include "DBManager.h"

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
		ts->read(c->ID, &role, sizeof(S_SELECT_ROLE));

		// 1.验证账号
		auto m = __UserManager->findMember(role.memid);
		if (m == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_200, 1011, &role, sizeof(S_SELECT_ROLE));
			return;
		}
		// 账号的状态必须是选择角色状态
		if (m->mem.state != M_SELECTROLE)
		{
			u16 childcmd = 1000 + m->mem.state;
			sendErrInfo(ts, c->ID, CMD_200, childcmd, &role, sizeof(S_SELECT_ROLE));
			return;
		}
		if (role.roleindex >= USER_MAX_ROLE)
		{
			sendErrInfo(ts, c->ID, CMD_200, 1012, &role, sizeof(S_SELECT_ROLE));
			return;
		}
		// 选择角色不能为空
		S_USER_MEMBER_ROLE* r = &m->role[role.roleindex];
		if (r->isT() == false)
		{
			sendErrInfo(ts, c->ID, CMD_200, 1013, &role, sizeof(S_SELECT_ROLE));
			return;
		}

		m->mem.state = M_LOGINING;

		auto db = __DBManager->GetDBSource(ETT_USERREAD);
		auto buff = db->PopBuffer();
		buff->b(CMD_200);
		buff->s(c->ID);
		buff->s(c->clientID);
		buff->s(m->mem.tableID);
		buff->s((s64)r->id);
		buff->s(&role, sizeof(S_SELECT_ROLE));
		buff->e();
		db->PushToThread(buff);
	}

	void onCreateRole(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_CREATE_ROLE role;
		ts->read(c->ID, &role, sizeof(S_CREATE_ROLE));

		auto m = __UserManager->findMember(role.memid);
		if (m == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_300, 1011, &role, sizeof(S_CREATE_ROLE));
			return;
		}
		// 账号的状态必须是选择角色状态
		if (m->mem.state != M_SELECTROLE)
		{
			u16 childcmd = 1000 + m->mem.state;
			sendErrInfo(ts, c->ID, CMD_300, childcmd, &role, sizeof(S_CREATE_ROLE));
			return;
		}
		// 是否有空闲位置可以创建角色
		s32 roleindex = m->findFreeRoleIndex();
		if (roleindex == -1)
		{
			sendErrInfo(ts, c->ID, CMD_300, 1012, &role, sizeof(S_CREATE_ROLE));
			return;
		}
		// 验证昵称是否重复
		bool isT = __UserManager->findNick(role.nick);
		if (isT)
		{
			sendErrInfo(ts, c->ID, CMD_300, 1013, &role, sizeof(S_CREATE_ROLE));
			return;
		}

		role.roleindex = roleindex;

		auto db = __DBManager->GetDBSource(ETT_USERWRITE);
		auto buff = db->PopBuffer();
		buff->b(CMD_300);
		buff->s(c->ID);
		buff->s(c->clientID);
		buff->s(m->mem.tableID);
		buff->s(&role, sizeof(S_CREATE_ROLE));
		buff->e();
		db->PushToThread(buff);
	}

	void onDeleteRole(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_DELETE_ROLE role;
		ts->read(c->ID, &role, sizeof(S_DELETE_ROLE));

		auto m = __UserManager->findMember(role.memid);
		if (m == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_400, 1011, &role, sizeof(S_DELETE_ROLE));
			return;
		}
		// 账号的状态必须是选择角色状态
		if (m->mem.state != M_SELECTROLE)
		{
			u16 childcmd = 1000 + m->mem.state;
			sendErrInfo(ts, c->ID, CMD_400, childcmd, &role, sizeof(S_DELETE_ROLE));
			return;
		}

		if (role.roleindex >= USER_MAX_ROLE)
		{
			sendErrInfo(ts, c->ID, CMD_400, 1012, &role, sizeof(S_DELETE_ROLE));
			return;
		}
		// 角色ID出错
		if (role.roleid < 10000)
		{
			sendErrInfo(ts, c->ID, CMD_400, 1013, &role, sizeof(S_DELETE_ROLE));
			return;
		}

		S_USER_MEMBER_ROLE* r = &m->role[role.roleindex];
		if (r->id != role.roleid)
		{
			sendErrInfo(ts, c->ID, CMD_400, 1014, &role, sizeof(S_DELETE_ROLE));
			return;
		}

		//删除缓存中的角色昵称
		__UserManager->deleteNick(r->nick);

		auto db = __DBManager->GetDBSource(ETT_USERWRITE);
		auto buff = db->PopBuffer();
		buff->b(CMD_400);
		buff->s(c->ID);
		buff->s(c->clientID);
		buff->s(m->mem.tableID);
		buff->s(&role, sizeof(S_DELETE_ROLE));
		buff->e();
		db->PushToThread(buff);
	}

	void onLoadRoleData(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u16 childcmd = 0;
		S_LOAD_ROLE role;
		ts->read(c->ID, childcmd);
		ts->read(c->ID, &role, sizeof(S_LOAD_ROLE));

		auto m = __UserManager->findMember(role.memid);
		if (m == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_500, 1011, &role, sizeof(S_LOAD_ROLE));
			return;
		}

		if (m->mem.state != M_LOGINING)
		{
			u16 childcmd = 1000 + m->mem.state;
			// m->mem.state = M_SELECTROLE;
			sendErrInfo(ts, c->ID, CMD_500, childcmd, &role, sizeof(S_LOAD_ROLE));
			return;
		}

		S_USER_ONLINE* user = __UserManager->findUser(role.userindex, role.memid);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_500, 1012, &role, sizeof(S_LOAD_ROLE));
			return;
		}

		m->mem.state = M_LOGINEND;
		m->mem.timeLastLogin = (int)time(NULL);

		user->mem.state = M_LOGINEND;
		user->mem.timeLastLogin = (int)time(NULL);
		user->tmp.temp_HeartTime = (int)time(NULL);
		user->tmp.temp_SaveTime = (int)time(NULL);

		__TcpServer->begin(c->ID, CMD_500);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, &role, sizeof(S_LOAD_ROLE));
		__TcpServer->sss(c->ID, &m->mem, sizeof(S_USER_MEMBER_DATA));
		__TcpServer->sss(c->ID, &user->role, sizeof(S_USER_ROLE));
		__TcpServer->end(c->ID);
	}

	//901 获取角色数据
	void onLoadRoledata_ChangeMap(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		u16 childcmd = 0;
		S_LOAD_ROLE data;
		ts->read(c->ID, childcmd);//0 加载角色数据 3000 切换地图
		ts->read(c->ID, &data, sizeof(S_LOAD_ROLE));

		// 1、验证账号
		auto m = __UserManager->findMember(data.memid);
		if (m == NULL)
		{
			sendErrInfo(ts, c->ID, cmd, 1011, &data, sizeof(S_LOAD_ROLE));
			return;
		}

		//2、验证状态
		if (m->mem.state != M_LOGINEND)
		{
			u16 childcmd = 1000 + m->mem.state;
			sendErrInfo(ts, c->ID, cmd, childcmd, &data, sizeof(S_LOAD_ROLE));
			return;
		}

		//3、验证用户
		auto user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, cmd, 1012, &data, sizeof(S_LOAD_ROLE));
			return;
		}
		if (childcmd == 3000)
		{
			__TcpServer->begin(c->ID, cmd);
			__TcpServer->sss(c->ID, (u16)3000);
			__TcpServer->sss(c->ID, &data, sizeof(S_LOAD_ROLE));
			__TcpServer->end(c->ID);
			return;
		}
		LOG_MESSAGE("loadrole data...%d \n", cmd);
		__TcpServer->begin(c->ID, cmd);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, &data, sizeof(S_LOAD_ROLE));
		__TcpServer->sss(c->ID, &m->mem, sizeof(S_USER_MEMBER_DATA));
		__TcpServer->sss(c->ID, &user->role, sizeof(S_USER_ROLE));
		__TcpServer->end(c->ID);
	}


	bool AppSelectRole::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		// 确定和客户端是安全连接
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppSelectRole err ... line:%d", __LINE__);
			return false;
		}

		// selectrole只应在center服务器发上来
		if (c->clientType != func::S_TYPE_CENTER && c->clientType != func::S_TYPE_GAME) return false;

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
		case CMD_500:
			onLoadRoleData(ts, c);		// 加载玩家角色数据
			break;
		case CMD_901:
		case CMD_902:
		case CMD_7200:
		case CMD_7300:
		case CMD_7400:
		case CMD_7301:
			onLoadRoledata_ChangeMap(ts, c, cmd);	// 加载玩家角色数据
			break;
		}
		return true;
	}

	/*******************************************************************************************/

	void onCMD_200(DBBuffer* buff)
	{
		u16 childcmd = 0;
		s32 connectID;
		s32 clientID;
		S_SELECT_ROLE role;


		buff->r(childcmd);
		buff->r(connectID);
		buff->r(clientID);
		buff->r(&role, sizeof(app::S_SELECT_ROLE));
		LOG_MESSAGE("200 mysql返回开始游戏.....memid:%lld/%d \n", role.memid, role.user_connectid);


		auto m = __UserManager->findMember(role.memid);
	
		// 查找连接
		auto c = __TcpServer->client(connectID, (u32)clientID);
		if (c == nullptr)
		{
			LOG_MESSAGE("onCMD_200 err... %d-%d\n", connectID, clientID);
			if (m != NULL) m->mem.state = M_SELECTROLE;
			return;
		}
		if (m == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_200, 1021, &role, sizeof(S_SELECT_ROLE));
			return;
		}
		if (childcmd != 0)
		{
			m->mem.state = M_SELECTROLE;
			sendErrInfo(__TcpServer, c->ID, CMD_200, childcmd, &role, sizeof(S_SELECT_ROLE));
			return;
		}

		// 验证都通过了
		if (m->mem.state != M_LOGINING)
		{
			childcmd = 1000 + m->mem.state;
			sendErrInfo(__TcpServer, c->ID, CMD_200, childcmd, &role, sizeof(S_SELECT_ROLE));
			return;
		}

		S_USER_ONLINE* user = __UserManager->findFreeUser();
		// 服务器满了
		if (user == nullptr)
		{
			m->mem.state = M_SELECTROLE;
			sendErrInfo(__TcpServer, c->ID, CMD_200, 1022, &role, sizeof(S_SELECT_ROLE));
			return;
		}

		// 读取角色数据
		buff->r(&user->role, sizeof(S_USER_ROLE));

		role.userindex = user->tmp.userindex;
		role.mapid = user->role.base.status.mapid;

		// 玩家角色数据
		user->tmp.server_connectid = role.server_connectid;
		user->tmp.server_clientid = role.server_clientid;
		user->tmp.user_connectid = role.user_connectid;
		user->tmp.temp_HeartTime = (int)time(NULL);
		user->tmp.temp_SaveTime = (int)time(NULL);
		// 账号数据
		m->tmp.roleindex = role.roleindex;
		m->tmp.userindex = user->tmp.userindex;

		memcpy(&user->mem, &m->mem, sizeof(S_USER_MEMBER_DATA));

		S_USER_MEMBER_ROLE* rolebase = &m->role[role.roleindex];
		LOG_MESSAGE("Login Success...%lld-%s  userindex:%d\n", 
			user->mem.id, user->mem.name, user->tmp.userindex);

		//初始化下新手
		if (user->role.base.exp.level == 0)
		{
			role.mapid = USER_BORN_MAPID;
		}

		__TcpServer->begin(c->ID, CMD_200);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, &role, sizeof(S_SELECT_ROLE));
// 		__TcpServer->sss(c->ID, &m->mem, sizeof(S_USER_MEMBER_DATA));
// 		__TcpServer->sss(c->ID, rolebase, sizeof(S_USER_MEMBER_ROLE));
// 		__TcpServer->sss(c->ID, &user->role.base.status, sizeof(S_ROLE_BASE_STATUS));
		__TcpServer->end(c->ID);

		auto db = __DBManager->GetDBSource(ETT_USERWRITE);
		auto buff2 = db->PopBuffer();
		buff2->b(CMD_30);
		buff2->s((s64)m->mem.id);
		buff2->e();
		db->PushToThread(buff2);
	}

	void onCMD_300(DBBuffer* buff)
	{
		u16 childcmd = 0;
		s32 connectid;
		s32 clientid;
		S_CREATE_ROLE role;

		buff->r(childcmd);
		buff->r(connectid);
		buff->r(clientid);
		buff->r(&role, sizeof(S_CREATE_ROLE));

		auto m = __UserManager->findMember(role.memid);
		if (m != NULL)
		{
			S_USER_MEMBER_ROLE* r = &m->role[role.roleindex];

			r->id = role.roleid;
			r->job = role.job;
			r->sex = role.sex;
			r->level = 0;
			r->clothid = 0;
			r->weaponid = 0;
			memcpy(r->nick, role.nick, USER_MAX_NICK);

			// 插入昵称
			__UserManager->insertNick(role.nick);
		}

		// 查找连接
		auto c = __TcpServer->client(connectid, clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("onCMD_300 err... %d-%d\n", connectid, clientid);
			return;
		}
		sendErrInfo(__TcpServer, c->ID, CMD_300, childcmd, &role, sizeof(S_CREATE_ROLE));
	}

	void onCMD_400(DBBuffer* buff)
	{
		u16 childcmd = 0;
		s32 connectid;
		s32 clientid;
		S_DELETE_ROLE role;

		buff->r(childcmd);
		buff->r(connectid);
		buff->r(clientid);
		buff->r(&role, sizeof(S_DELETE_ROLE));

		auto m = __UserManager->findMember(role.memid);
		if (m != NULL)
		{
			S_USER_MEMBER_ROLE* r = &m->role[role.roleindex];
			r->reset();
		}

		// 查找连接
		auto c = __TcpServer->client(connectid, clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("onCMD_400 err... %d-%d\n", connectid, clientid);
			return;
		}
		//返回删除角色结果
		sendErrInfo(__TcpServer, c->ID, CMD_400, childcmd, &role, sizeof(S_DELETE_ROLE));
	}

	bool AppSelectRole::onDBCommand(void*_buff, const u16 cmd)
	{
		DBBuffer* buff = (DBBuffer*)_buff;
		if (buff == nullptr) return false;

		switch (cmd)
		{
		case CMD_200:
			onCMD_200(buff);
			break;
		case CMD_300:
			onCMD_300(buff);
			break;
		case CMD_400:
			onCMD_400(buff);
			break;
		}
		return true;
	}
}



