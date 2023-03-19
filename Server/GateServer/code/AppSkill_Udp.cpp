#include "AppSkill.h"


#include "CommandData.h"
#include "UserManager.h"
#include "AppGlobal.h"

#ifdef TEST_UDP_SERVER

using namespace net;

namespace app
{
	AppSkill_Udp* __AppSkilludp = NULL;

	AppSkill_Udp::AppSkill_Udp()
	{
	}

	AppSkill_Udp::~AppSkill_Udp()
	{
	}

	//3000使用技能
	void onSkill_Udp(net::IUdpServer* udp, net::S_UDP_BASE* c)
	{
		S_SKILL_3000 data3000;
		udp->readData(c->ID, &data3000, sizeof(S_SKILL_3000));


		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_3000, 4001);
			return;
		}

		S_SKILL_ROLE data;
		data.user_connectid = c->ID;
		data.memid = c->memid;
		data.userindex = user->userindex;
		data.skillindex = data3000.skillindex;
		data.lock_type = data3000.lock_type;
		data.lock_index = data3000.lock_index;
		data.targetpos = data3000.pos;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_3000, 4002);
			return;
		}

		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_3000, 4003);
			return;
		}


		LOG_MESSAGE("udp use skill...%lld\n", c->memid);


		//发送
		tcp->begin(CMD_3000);
		tcp->sss(&data, sizeof(S_SKILL_ROLE));
		tcp->end();
	}

	bool AppSkill_Udp::onUDPServerCommand(net::IUdpServer* udp, net::S_UDP_BASE* c, const u16 cmd)
	{
		if (c->memid < 1) return false;
		auto tcp_c = __TcpServer->client(c->ID);
		if (tcp_c == NULL || tcp_c->memid != c->memid) return false;
		if (tcp_c->state < func::S_LOGIN) return false;

		switch (cmd)
		{
		case CMD_3000:
			onSkill_Udp(udp, c);
			break;
		}
		return false;
	}


	//******************************************************************
	//******************************************************************
	//******************************************************************
	//收到gameserver技能同步消息
	void onCMD_3100_Udp(net::ITCPClient* tc)
	{
		s32 skillid = 0;
		u8  skilllevel = 0;
		S_SKILL_ROLE data;

		tc->read(data.user_connectid);
		tc->read(data.memid);
		tc->read(data.userindex);
		tc->read(skillid);
		tc->read(skilllevel);
		tc->read(data.lock_type);
		tc->read(data.lock_index);
		tc->read(&data.targetpos, sizeof(S_VECTOR3));

		LOG_MESSAGE("udp AppSkill %d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);


		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppSkill user == NULL...%d-%lldd line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}

		//使用UDP协议同步技能
		auto c = user->connection;

		S_SKILL_3100  data3100;
		data3100.ID = c->ID;
		data3100.cmd = CMD_3100;
		data3100.userindex = data.userindex;
		data3100.skillid = skillid;
		data3100.level = skilllevel;
		data3100.lock_type = data.lock_type;
		data3100.lock_index = data.lock_index;
		data3100.pos = data.targetpos;
		

		data3100.setSecure(func::__UDPServerInfo->RCode);
		__IUdpServer->sendData(c->ID, &data3100, sizeof(S_SKILL_3100), net::SPT_UDP);
	}
}

#endif