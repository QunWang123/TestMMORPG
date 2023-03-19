#include "AppSelectRole.h"
#include "UserManager.h"
#include "WorldData.h"
#include "AppGlobal.h"
#include "WorldBC.h"

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

	// �յ��������������
	void onHeart(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex = 0;
		u64 memid;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		auto user = __UserManager->findUser(userindex, memid);

		if (user == NULL)
		{
			LOG_MESSAGE("heart err ...%d-%lld\n", userindex, memid);
			return;
		}

		user->tmp.temp_HeartTime = (int)time(NULL);
	}

	void onSelectRole(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_LOAD_ROLE* data = __UserManager->popRequestPool();
		ts->read(c->ID, data, sizeof(S_LOAD_ROLE));
		data->cmd = CMD_500;
		data->server_connectid = c->ID;
		data->server_clientid = c->clientID;
		
		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_500, 3001, data, sizeof(S_LOAD_ROLE));
			// data->reset();	
			__UserManager->pushRequestPool(data);
			return;
		}
		__UserManager->insertRequest(data);
	}

	// ��ҵ���
	void onUserDisconnect(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex = 0;
		u64 memid;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		auto user = __UserManager->findUser(userindex, memid);

		if (user == NULL)
		{
			LOG_MESSAGE("on user disconnect err ...%d-%lld\n", userindex, memid);
			TS_Broadcast::db_Disconnect(userindex, memid, 0);
			return;
		}
		LOG_MESSAGE("user disconnect ...%lld-%d\n", memid, __LINE__);
		
		__UserManager->userDisconnect(user);
	}

	bool AppSelectRole::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppSelectRole err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_90:
			onHeart(ts, c);
			break;
		case CMD_500:
			onSelectRole(ts, c);
			break;
		case CMD_600:
			onUserDisconnect(ts, c);
			break;
		}
		return true;
	}

	/*******************************************************************************************/
	void onCMD_500(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_LOAD_ROLE data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_LOAD_ROLE));

		auto c = __TcpServer->client(data.server_connectid, data.server_clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("AppSelectRole err c == NULL [%d-%lld] line:%d\n",
				data.server_connectid, data.server_clientid, __LINE__);
			return;
		}
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_500, childcmd, &data, sizeof(S_LOAD_ROLE));
			return;
		}
		// ��֤��λ��û����
		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user != NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_500, 3011, &data, sizeof(S_LOAD_ROLE));
			return;
		}
		// ������������
		user = __UserManager->popUser();
		user->reset();
		// ������Ϸ��������maxuserҲӦ�Ǻ�DB�����
		bool isok = __UserManager->insertUser(data.userindex, user);
		if (!isok)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_500, 3012, &data, sizeof(S_LOAD_ROLE));
			return;
		}

		// ��ȡ����
		tc->read(&user->mem, sizeof(S_USER_MEMBER_DATA));
		// ����ֱ�ӽ���S_USER_ROLEû����
		// DB������Ĵ�С��һ����
		// ����DB����Ʒ��scriptָ�����NULL
		// ����Ӱ����GameServer�е�ʹ��
		tc->read(&user->role, sizeof(S_USER_ROLE));
		user->tmp.userindex = data.userindex;
		user->tmp.server_clientid = c->clientID;
		user->tmp.server_connectid = c->ID;
		user->tmp.user_connectid = data.user_connectid;
		user->tmp.temp_HeartTime = (int)time(NULL);

		LOG_MESSAGE("load role... %lld-%d \n",
			user->mem.id, user->role.base.innate.job);
		for (int i = 0; i < MAX_SKILL_COUNT; i++)
		{
			auto skill = &user->role.stand.myskill.skill[i];
			LOG_MESSAGE("load role skill... %d-%d \n",
				skill->id, skill->level);
		}
		
		// user->role.base.life.hp = 1000;
		// user->role.base.life.mp = 1000;

		// ��ʼ���ڵ�����
		user->node.init(E_NODE_TYPE::N_ROLE, data.userindex, -1);

		// �����ǲ�������
		if (user->role.base.exp.level == 0)
		{
			user->initNewUser();
			data.mapid = user->role.base.status.mapid;
			LOG_MESSAGE("init new user %lld \n", user->mem.id);
			// �ش���DB
			__TcpDB->begin(CMD_610);
			__TcpDB->sss(user->tmp.userindex);
			__TcpDB->sss(user->mem.id);
			__TcpDB->sss(&user->role.base, sizeof(S_ROLE_BASE));
			__TcpDB->sss(&user->role.stand.myskill, sizeof(S_ROLE_STAND_SKILL));
			__TcpDB->sss(&user->role.stand.atk, sizeof(S_SPRITE_ATTACK));
			__TcpDB->sss(user->role.stand.bag.num);
			// __TcpDB->sss(user->role.stand.warehouse.num);
			__TcpDB->end();
		}
		//�����������
		user->updateAtk(true);
		//���±������߽ű���ս��װ���ű�
		user->updateBagScript();
		user->updateCombatScript();
		__UserManager->setOnlineCount(true);
		__TcpServer->begin(c->ID, CMD_500);
		__TcpServer->sss(c->ID, childcmd);
		__TcpServer->sss(c->ID, &data, sizeof(S_LOAD_ROLE));
		// __TcpServer->sss(c->ID, &user->role, sizeof(S_USER_ROLE));
		__TcpServer->sss(c->ID, &user->role.base, sizeof(S_ROLE_BASE));
		__TcpServer->sss(c->ID, &user->role.stand.myskill, sizeof(S_ROLE_STAND_SKILL));
		__TcpServer->end(c->ID);

		TS_Broadcast::do_SendAtk(user, user);
	}

	bool AppSelectRole::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_DB) return false;

		switch (cmd)
		{
		case CMD_500:
			onCMD_500(tc);
			break;
		}
		return true;
	}
}



