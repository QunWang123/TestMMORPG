#include "UserManager.h"
#include "AppGlobal.h"
#include "CommandData.h"
#include "ShareFunction.h"
#include "CommandData2.h"

namespace app
{
	UserManager* __UserManager = nullptr;

	UserManager::UserManager()
	{
		__onlineUsers = nullptr;
		Init();
	}

	UserManager::~UserManager()
	{

	}

	void UserManager::Init()
	{
		// ���������û��ģ������Ϸ��������
		__onlineUsers = new HashArray<S_USER_GATE>(func::__ServerInfo->MaxConnect);
		for (int i = 0; i < __onlineUsers->length; i++)
		{
			S_USER_GATE* user = __onlineUsers->Value(i);
			user->reset();
		}
	}

	//����Game������� ����ȫ���������
	void onCheckDisconnect_Game()
	{
		LOG_MESSAGE("disconnect GameServer closeClient...\n");

		//1������Ͽ����ӵ�gameserver�ϵ�ȫ���������
		int count = func::__ServerInfo->MaxUser;
		for (int i = 0; i < count; i++)
		{
			auto user = __UserManager->findUser(i);
			if (user == nullptr)  continue;
			if (user->state != M_LOGINEND) continue;
			if (user->memid < 10000) continue;

			auto tcp = user->tcpGame;
			if (tcp == nullptr) continue;
			if (tcp->getData()->state >= func::C_CONNECT_SECURE)  continue;

			auto c = user->connection;
			if (c == nullptr) continue;

			LOG_MESSAGE("closeClient gate disconnect Game...%lld-%d\n", user->memid, user->userindex);
			// io::pushLog(io::EFT_ERR, "closeClient gate disconnect Game...%lld-%d\n", user->memid, user->userindex);
			
			__TcpServer->closeClient(c->ID);
		}
	}

	void UserManager::update()
	{
		//1������ǰ��Ϊ�ͻ��� ���ߵ���������Ͽ������ķ���������Ϸ�������ȵ����ӣ�
		if (!__TcpClientDisconnets.empty())
		{
			int num = 0;
			while (true)
			{
				u8 server_type = 0;
				__TcpClientDisconnets.try_pop(server_type);

				switch (server_type)
				{
				case func::S_TYPE_GAME:
					onCheckDisconnect_Game();
					break;
					//case func::S_TYPE_TEAM:
					//	//onCheckDisconnect_Team();
					//	break;
				}
				if (__TcpClientDisconnets.empty()) break;
				//������ѭ��
				++num;
				if (num >= 10) break;
			}
		}
	}

	app::S_USER_GATE* UserManager::findUser(const u32 connectid)
	{
		if (connectid >= __onlineUsers->length) return nullptr;
		return __onlineUsers->Value(connectid);
	}

	app::S_USER_GATE* UserManager::findUser(const u32 connectid, const u64 memid)
	{
		auto user = findUser(connectid);
		if (user == nullptr || user->memid != memid) return nullptr;
		return user;
	}

	app::S_USER_GATE* UserManager::findUser_Connection(const u32 connectid, const u64 memid)
	{
		auto user = findUser(connectid, memid);
		if (user == nullptr || 
			user->connection == nullptr || 
			user->connection->memid != memid) return nullptr;
		return user;
	}

	//������Ϸ��ͼID����������Ҫ�����Ǹ���Ϸ������
	net::ITCPClient* UserManager::findGameServer_Connection(u8 line, u32 mapid)
	{
		u32 serverid = share::findGameServerID(line, mapid);
		int size = __TcpGame.size();
		for (int i = 0; i < size; i++)
		{
			auto tcp = __TcpGame[i];
			if (tcp->getData()->serverID == serverid)
			{
				return tcp;
			}
		}
		return nullptr;
	}

	//********************************************************************
	//********************************************************************
	//********************************************************************


	//���͸����������[chat��team]-��ҽ���
	void UserManager::sendWorld_UserEntryWorld(net::ITCPClient* tcp, S_USER_GATE* user)
	{
		if (tcp == NULL) return;

		S_CMD_USER_ENTRY data;
		data.userindex = user->userindex;
		if (user->connection != NULL) data.user_connectid = user->connection->ID;
		data.memid = user->memid;
		data.mapid = user->mapid;
		data.line = user->line;
		data.level = user->level;
		memcpy(data.name, user->name, USER_MAX_MEMBER);
		memcpy(data.nick, user->nick, USER_MAX_NICK);

		tcp->begin(CMD_900);
		tcp->sss(&data, sizeof(S_CMD_USER_ENTRY));
		tcp->end();
	}
	//���͸����������[chat��team]-��ҽ���
	void UserManager::sendWorld_UserEntryWorld(net::ITCPClient* tcp)
	{
		if (tcp == NULL) return;

		s32 length = __UserManager->__onlineUsers->length;
		for (int i = 0; i < length; i++)
		{
			S_USER_GATE* user = __UserManager->findUser(i);
			if (user == nullptr) continue;
			if (user->state != M_LOGINEND)   continue;
			if (user->connection == nullptr) continue;
			if (user->memid < 10000) continue;
			if (user->mapid < 1)     continue;

			sendWorld_UserEntryWorld(tcp, user);
		}
	}

	void UserManager::sendWorld_UserEntryWorld(S_USER_GATE* user)
	{
		this->sendWorld_UserEntryWorld(__TcpCenter, user);
		//this->sendWorld_UserEntryWorld(__TcpCenter,user);
	}

	//�ͷ��������ӳɹ� ����ȫ�������������
	void UserManager::sendWorld_UserEntryWorld()
	{
		this->sendWorld_UserEntryWorld(__TcpCenter);
		//this->sendWorld_UserEntryWorld(__TcpCenter);
	}

	//****************************************************************
	//****************************************************************
	//****************************************************************
	//��¼ ֪ͨgame chat team xx����
	void UserManager::sendWorld_UserLeaveWorld(S_USER_GATE* user)
	{
		this->sendWorld_UserLeaveWorld(__TcpCenter, user);//1��֪ͨ���ķ����� xx��ҵ���
		this->sendWorld_UserLeaveWorld(user->tcpGame, user);//2��֪ͨ��Ϸ������ xx��ҵ��� 
		//this->sendWorld_UserLeaveWorld(__TcpChat, user);//3��֪ͨ��������� xx��ҵ���
		//this->sendWorld_UserLeaveWorld(__TcpTeam, user);//4��֪ͨ��ӷ����� xx��ҵ���
	}

	//��û�е�¼ ֻ֪ͨ���ķ�����xx ���ӵ���
	void UserManager::sendWorld_UserLeaveWorld(s64 memid)
	{
		__TcpCenter->begin(CMD_600);
		__TcpCenter->sss(-1);
		__TcpCenter->sss(memid);
		__TcpCenter->end();
	}

	//���͸����������[chat��team]-����뿪
	void UserManager::sendWorld_UserLeaveWorld(net::ITCPClient* tcp, S_USER_GATE* user)
	{
		if (tcp == NULL) return;
		tcp->begin(CMD_600);
		tcp->sss(user->userindex);
		tcp->sss(user->memid);
		tcp->end();
	}

}

