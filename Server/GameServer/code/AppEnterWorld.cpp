#include "AppEnterWorld.h"
#include "CommandData.h"
#include "WorldData.h"
#include "UserManager.h"
#include "AppGlobal.h"
#include "WorldBC.h"
#include "ShareFunction.h"
namespace app
{
	AppEnterWorld* __AppEntry = nullptr;

	AppEnterWorld::AppEnterWorld()
	{

	}

	AppEnterWorld::~AppEnterWorld()
	{

	}

	void AppEnterWorld::onInit()
	{

	}

	// CMD_700
	void onGetOtherRoleData(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_GETROLE_DATA data;
		ts->read(c->ID, &data, sizeof(S_GETROLE_DATA));

		auto targetuser = __UserManager->findUser(data.targetindex);
		if (targetuser == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_700, 3001, &data, sizeof(S_GETROLE_DATA));
			return;
		}

		__TcpServer->begin(c->ID, CMD_700);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, &data, sizeof(S_GETROLE_DATA));
		__TcpServer->sss(c->ID, &targetuser->role.base.innate, C_ROLE_INNATE_LEN);
		__TcpServer->sss(c->ID, &targetuser->role.base.status, C_ROLE_STATUS_LEN);
		__TcpServer->sss(c->ID, &targetuser->role.base.exp, C_ROLE_EXP_LEN);
		__TcpServer->sss(c->ID, &targetuser->role.base.life, sizeof(S_ROLE_BASE_LIFE));
		__TcpServer->end(c->ID);
		
		// ��ȡ������ʱ��˳���buffҲ������ȥ
		for (u32 i = 0; i < MAX_BUFF_COUNT; i++)
		{
			S_TEMP_BUFF_RUN_BASE* run = &targetuser->tmp.temp_BuffRun.data[i];
			if (run->buff_id <= 0) continue;
			if (run->runningtime <= 200) continue;
			TS_Broadcast::do_SendBuff(CMD_5000, c->ID, data.user_connectid, data.memid, targetuser->node.index, run);
		}
	}

	void onEntryWorld(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_ENTRY_BASE data;
		ts->read(c->ID, &data, sizeof(S_ENTRY_BASE));

		// ��selectrole�е�onCMD_500�в����
		auto user = __UserManager->findUser(data.userindex, data.memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_900, 3001, &data, sizeof(S_GETROLE_DATA));
			return;
		}

		user->enterWorld();
	}


	//901 �л���ͼ
	//902 ����
	void onChangeMap(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		u16 childcmd = 0;
		S_CHANGEMAP_BASE data;
		ts->read(c->ID, childcmd);
		ts->read(c->ID, &data, sizeof(S_CHANGEMAP_BASE));

		// ����·�Ͳ�������µ��е�ͼ������gameserver������£�
		//							DBServer											DBServer
		//			�ڡ�childcmd=3000	 �ۡ�childcmd=3000(ԭ������)		�ޡ�childcmd=0		
		//			ԭgameserver			ԭgameserver				��ҵ���gameserver
		// �١�childcmd=0						�ܡ�childcmd=3000			�ݡ�childcmd=1	
		// gateserver						gateserver(�����������gameserver)
		//  ��902��901��
		// ���

		// ������û�gameserver�������ڶ�����������

		//1����֤���
		S_USER_BASE* user = nullptr;
		if (childcmd == 1) //���ؽ�ɫ��ɫ����
		{
			// 
			//1����֤���������Ч�� �õ�ͼ�Ѿ���������� ����
			user = __UserManager->findUser(data.userindex);
			if (user != nullptr)
			{
				sendErrInfo(ts, c->ID, cmd, 3001, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
		}
		else
		{
			//1����֤���������Ч�� ������Ψһ����+�˺�ID
			user = __UserManager->findUser(data.userindex, data.memid);
			if (user == nullptr)
			{
				sendErrInfo(ts, c->ID, cmd, 3001, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
		}

		//2����֤DB����
		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, cmd, 3002, &data, sizeof(S_CHANGEMAP_BASE));
			return;
		}

		//������ؽ�ɫ����
		if (childcmd == 1)
		{
			__UserManager->onPushLoadRole(c, &data, data.userindex, data.mapid, cmd, childcmd);
			return;
		}

		//�л���ͼ
		if (cmd == CMD_901)
		{
			//3����֤��ͼID��Ч��
			if (data.mapid >= MAX_MAP_ID)
			{
				sendErrInfo(ts, c->ID, cmd, 3003, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}

			//4��Ҫȥ�ĵ�ͼID�����ڵ���һ����
			if (data.mapid == user->role.base.status.mapid)
			{
				sendErrInfo(ts, c->ID, cmd, 3004, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
		}
		else //�л���·
		{
			//3��Ҫȥ����·�����ڵ���һ����
			if (data.line == share::__ServerLine)
			{
				sendErrInfo(ts, c->ID, cmd, 3003, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
			//4����֤��ͼID��Ч��
			if (data.mapid != user->role.base.status.mapid)
			{
				sendErrInfo(ts, c->ID, cmd, 3004, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
		}


		//��ȡ��Ҫ�л���ͼ���ڵķ�����ID
		//Ҫȥ�ķ��������ǵ�ǰ���ڷ����� �������л�
		//���ߵĻ� ��������������
		u32 serverid = share::findGameServerID(share::__ServerLine, data.mapid);
		if (serverid == func::__ServerInfo->ID)
		{
			// ��������˵������ͬһ��gameserver����ͼ��ֻ����²������ݼ���
			// �л���·�϶�������ͬһ������
			if (cmd == CMD_902)
			{
				sendErrInfo(ts, c->ID, cmd, 3005, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}

			user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_NO);

			user->role.base.status.mapid = data.mapid;
			user->initBornPos(data.mapid);
			user->role.base.status.pos.z = 0;
			
			//֪ͨ��ӷ����� ����л���ͼ
			TS_Broadcast::team_ChangeMap(user, CMD_901);
			//�����л���ͼ�ɹ�
			TS_Broadcast::do_ChangeMap(user, &data, sizeof(S_CHANGEMAP_BASE), c->ID, cmd);

			LOG_MESSAGE("changeMap success...%d \n", data.mapid);
			return;
		}
		// �ߵ���˵�����������
		// ��ͬһ��ͼҪ����
		// ��ͬһ��Ҫ��ͼ���µ��л�gameserver

		LOG_MESSAGE("changeMap start.....index:%d %lld %d \n", data.userindex, data.memid, data.mapid);

		// ֪ͨDB������ ��ʼ�л���ͼ  �����Ǹ��޿� һ��Ҫע��:
		// ����һ��Ҫ֪ͨDB �����DB���سɹ���Ϣ ���ܽ����л�  
		// ��ΪҪȷ����ҵ����ݶ�ͬ������DB ���ܽ�����һ������
		// �磺����������Ȼ��ʼѡ���ߣ����ʱ������������ݻ�ûͬ����DB 
		// ���ݻ��ڻ����� �п���DB������IO����æ�����ʱ���ڻ������
		// ���ʱ���ֱ�ӻ��� �л����������̺� ȥLOAD���� 
		// �п��ܼ��س������������� ���Ա���Ҫȷ��DB����ͬ���������������
		// ��ΪTCP��������� ���Է������������Ϣ ��˵����ȫ��ͬ�������
		user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_YES);

		TS_Broadcast::db_ChangeMap(c, &data, data.userindex, data.line, data.mapid, cmd, 3000);
	}

	bool AppEnterWorld::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppEnterWorld err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_700:
			onGetOtherRoleData(ts, c);
			break;
		case CMD_900:
			onEntryWorld(ts, c);
			break;
		case CMD_901: //�л���ͼ �� ���� һ���߼�
		case CMD_902:
			onChangeMap(ts, c, cmd); 
			break; //�л���·
		}
		return true;
	}

	//****************************************************************************
	//****************************************************************************
	//901 902 �յ�DB���������صļ��ؽ�ɫ���� ����DB����ͬ�����
	void onCMD_901(net::ITCPClient* tc, const u16 cmd)
	{
		u16 childcmd = 0;
		S_LOAD_ROLE data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_LOAD_ROLE));

		S_CHANGEMAP_BASE data2;
		data2.userindex = data.userindex;
		data2.memid = data.memid;
		data2.user_connectid = data.user_connectid;
		data2.mapid = data.mapid;
		data2.line == data.line;

		auto c = __TcpServer->client(data.server_connectid, data.server_clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("onCMD_%d c == NULL...%d %d line:%d \n", cmd, data.server_connectid, data.server_clientid, __LINE__);
			return;
		}
		if (childcmd == 3000)
		{
			//���ظ����ط����� ��Ҫ�л������������
			sendErrInfo(__TcpServer, c->ID, cmd, 3000, &data2, sizeof(S_CHANGEMAP_BASE));
			return;
		}
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, childcmd, &data2, sizeof(S_CHANGEMAP_BASE));
			return;
		}

		//1����֤���������Ч��
		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user != NULL)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3020, &data2, sizeof(S_CHANGEMAP_BASE));
			return;
		}
		//2��������������
		user = __UserManager->popUser();
		user->reset();
		bool isok = __UserManager->insertUser(data.userindex, user);
		if (isok == false)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3021, &data2, sizeof(S_CHANGEMAP_BASE));
			return;
		}

		//1����ȡ����
		tc->read(&user->mem, sizeof(S_USER_MEMBER_DATA));
		tc->read(&user->role, sizeof(S_USER_ROLE));

		//2��ֻ���л���ͼ�Ż����������� ���߲����������� ֱ��ʹ��ԭ������
		if (cmd == CMD_901)
		{
			//2���л���ͼ ��Ҫ����������
			s32 kind = user->initBornPos(data.mapid);
			if (kind != 0)
			{
				sendErrInfo(__TcpServer, c->ID, cmd, 3022, &data2, sizeof(S_CHANGEMAP_BASE));
				return;
			}
			user->role.base.status.mapid = data.mapid;
		}


		user->tmp.userindex = data.userindex;
		user->tmp.server_connectid = c->ID;
		user->tmp.server_clientid = c->clientID;
		user->tmp.user_connectid = data.user_connectid;
		user->tmp.temp_HeartTime = (int)time(NULL);
		
		//�л���ͼ�ɹ�
		user->changeMap(&data2, sizeof(S_CHANGEMAP_BASE), c->ID, cmd, ECK_SUCCESS);
		//֪ͨ��ӷ����� ����л���ͼ
		TS_Broadcast::team_ChangeMap(user, CMD_901);
		//�����л���ͼ�ɹ�
		TS_Broadcast::do_ChangeMap(user, &data2, sizeof(S_CHANGEMAP_BASE), c->ID, cmd);

		LOG_MESSAGE("changemap success........................\n");

	}

	bool AppEnterWorld::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_DB) return false;


		switch (cmd)
		{
		case CMD_901:
		case CMD_902:
			onCMD_901(tc, cmd);
			break;
		}

		return false;
	}
}



