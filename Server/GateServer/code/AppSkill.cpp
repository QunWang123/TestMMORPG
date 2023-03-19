#include "AppSkill.h"
#include "CommandData.h"
#include "UserManager.h"
#include "AppGlobal.h"
namespace app
{
	AppSkill* __AppSkill = nullptr;

	AppSkill::AppSkill()
	{

	}

	AppSkill::~AppSkill()
	{

	}

	void onSkill(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_SKILL_ROLE data;
		ts->read(c->ID, data.skillindex);
		ts->read(c->ID, data.lock_type);
		ts->read(c->ID, data.lock_index);
		ts->read(c->ID, &data.targetpos, sizeof(S_VECTOR3));

		S_USER_GATE* user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_3000, 4001);
			return;
		}

		data.user_connectid = c->ID;
		data.memid = c->memid;
		data.userindex = user->userindex;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_3000, 4002);
			return;
		}
		if (tcp->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_3000, 4003);
			return;
		}

		tcp->begin(CMD_3000);
		tcp->sss(&data, sizeof(S_SKILL_ROLE));
		tcp->end();
	}

	void onReborn(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_REBORN_ROLE data;
		ts->read(c->ID, data.kind);

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_4100, 4001);
			return;
		}

		data.user_connectid = c->ID;
		data.memid = c->memid;
		data.userindex = user->userindex;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_4100, 4002);
			return;
		}
		if (tcp->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, CMD_4100, 4003);
			return;
		}

		tcp->begin(CMD_4100);
		tcp->sss(&data, sizeof(S_REBORN_ROLE));
		tcp->end();
	}

	bool AppSkill::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppMove err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_USER || c->state != func::S_LOGIN) return false;

		switch (cmd)
		{
		case CMD_3000:
			onSkill(ts,c);
			break;
		case CMD_4100:
			onReborn(ts, c);
			break;
		}
		return true;
	}

	void onCMD_3000(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_SKILL_ROLE data;

		tc->read(childcmd);
		tc->read(&data, sizeof(S_SKILL_ROLE));

		S_USER_GATE* user = __UserManager->findUser(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSkill err ... user == null %d-%lld %d\n",
				data.user_connectid, data.memid, __LINE__);
			return;
		}
		auto c = user->connection;
		sendErrInfo(__TcpServer, c->ID, CMD_3000, childcmd);
	}

	void onCMD_3100(net::ITCPClient* tc)
	{
#ifdef TEST_UDP_SERVER
		onCMD_3100_Udp(tc);
		return;
#endif // DEBUG

		s32 skillid = 0;
		u8	skilllevel = 0;
		S_SKILL_ROLE data;

		tc->read(data.user_connectid);
		tc->read(data.memid);
		tc->read(data.userindex);
		tc->read(skillid);
		tc->read(skilllevel);
		tc->read(data.lock_type);
		tc->read(data.lock_index);
		tc->read(&data.targetpos, sizeof(S_VECTOR3));

		S_USER_GATE* user = __UserManager->findUser(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSkill err ... user == null %d-%lld %d\n",
				data.user_connectid, data.memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_3100);
		__TcpServer->sss(c->ID, data.userindex);
		__TcpServer->sss(c->ID, skillid);
		__TcpServer->sss(c->ID, skilllevel);
		__TcpServer->sss(c->ID, data.lock_type);
		__TcpServer->sss(c->ID, data.lock_index);
		__TcpServer->sss(c->ID, &data.targetpos, sizeof(S_VECTOR3));
		__TcpServer->end(c->ID);
	}
	// 玩家受伤
	void onCMD_3200(net::ITCPClient* tc)
	{
		S_SKILL_ROLE	data;
		s32				atk;
		s32				curhp;
		tc->read(data.user_connectid);
		tc->read(data.memid);
		tc->read(data.userindex);			// 发出技能的人
		tc->read(data.lock_type);
		tc->read(data.lock_index);		// 下标索引
		tc->read(curhp);					// 受伤害得玩家或者怪物或者其他
		tc->read(atk);						// 产生得生命值

		S_USER_GATE* user = __UserManager->findUser(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSkill err ... user == null %d-%lld %d\n",
				data.user_connectid, data.memid, __LINE__);
			return;
		}

		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_3200);
		__TcpServer->sss(c->ID, data.userindex);
		__TcpServer->sss(c->ID, data.lock_type);
		__TcpServer->sss(c->ID, data.lock_index);
		__TcpServer->sss(c->ID, curhp);
		__TcpServer->sss(c->ID, atk);
		__TcpServer->end(c->ID);
	}
	// 玩家死亡
	void onCMD_4000(net::ITCPClient* tc)
	{
		s32 connectid = 0;
		s64 memid = 0;
		s32 userindex = 0;
		tc->read(connectid);
		tc->read(memid);

		tc->read(userindex);
		S_USER_GATE* user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSkill err ... user == null %d-%lld %d\n",
				connectid, memid, __LINE__);
			return;
		}
		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_4000);
		__TcpServer->sss(c->ID, userindex);
		__TcpServer->end(c->ID);
	}

	void onCMD_4100(net::ITCPClient* tc)
	{
		u16 childcmd;
		S_REBORN_ROLE data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_REBORN_ROLE));

		S_USER_GATE* user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppReborn err ... user == null %d-%lld %d\n",
				data.user_connectid, data.memid, __LINE__);
			return;
		}

		auto c = user->connection;
		sendErrInfo(__TcpServer, c->ID, CMD_4100, childcmd);
	}

	void onCMD_4200(net::ITCPClient* tc)
	{
		s32 connectid = 0;
		s64 memid = 0;
		u32 curhp = 0;
		s32 userindex = 0;
		S_VECTOR3 pos;
		tc->read(connectid);//接收广播的连接ID
		tc->read(memid);

		tc->read(userindex);
		tc->read(curhp);
		tc->read(&pos, sizeof(S_VECTOR3));

		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSkill 4200 user == NULL...%d-%lldd line:%d \n", connectid, memid, __LINE__);
			return;
		}
		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_4200);
		__TcpServer->sss(c->ID, userindex);
		__TcpServer->sss(c->ID, curhp);
		__TcpServer->sss(c->ID, &pos, sizeof(S_VECTOR3));
		__TcpServer->end(c->ID);
	}

	void onCMD_5000(net::ITCPClient* tc)
	{
		s32 connectid;
		s64 memid;
		s32 buff_userindex;
		s32 buff_id;
		s32 buff_runningtime;
		tc->read(connectid);
		tc->read(memid);
		tc->read(buff_userindex);
		tc->read(buff_id);
		tc->read(buff_runningtime);

		S_USER_GATE* user = __UserManager->findUser(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSkill err ... user == null %d-%lld %d\n",
				connectid, memid, __LINE__);
			return;
		}
		auto c = user->connection;

		__TcpServer->begin(c->ID, CMD_5000);
		__TcpServer->sss(c->ID, buff_userindex);
		__TcpServer->sss(c->ID, buff_id);
		__TcpServer->sss(c->ID, buff_runningtime);
		__TcpServer->end(c->ID);
	}

	bool AppSkill::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_GAME) return false;

		switch (cmd)
		{
		case CMD_3000:
			onCMD_3000(tc);
			break;
		case CMD_3100:
			onCMD_3100(tc);
			break;
		case CMD_3200:
			onCMD_3200(tc);
			break;
		case CMD_4000:
			onCMD_4000(tc);
			break;
		case CMD_4100:
			onCMD_4100(tc);
			break;
		case CMD_4200:
			onCMD_4200(tc);
			break;
		case CMD_5000:
			onCMD_5000(tc);
			break;
		}
		return true;
	}

}