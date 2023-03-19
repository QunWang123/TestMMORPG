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
		// 创建连接用户的，玩家游戏基础数据
		__onlineUsers = new HashArray<S_USER_GATE>(func::__ServerInfo->MaxConnect);
		for (int i = 0; i < __onlineUsers->length; i++)
		{
			S_USER_GATE* user = __onlineUsers->Value(i);
			user->reset();
		}
	}

	//检查和Game连接情况 清理全部玩家连接
	void onCheckDisconnect_Game()
	{
		LOG_MESSAGE("disconnect GameServer closeClient...\n");

		//1、清理断开连接的gameserver上的全部玩家数据
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
		//1、处理当前做为客户端 掉线的情况（即断开和中心服务器、游戏服务器等的连接）
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
				//避免死循环
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

	//根据游戏地图ID来查找你需要进入那个游戏服务器
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


	//发送给世界服务器[chat、team]-玩家进入
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
	//发送给世界服务器[chat、team]-玩家进入
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

	//和服务器连接成功 发送全部在线玩家数据
	void UserManager::sendWorld_UserEntryWorld()
	{
		this->sendWorld_UserEntryWorld(__TcpCenter);
		//this->sendWorld_UserEntryWorld(__TcpCenter);
	}

	//****************************************************************
	//****************************************************************
	//****************************************************************
	//登录 通知game chat team xx掉线
	void UserManager::sendWorld_UserLeaveWorld(S_USER_GATE* user)
	{
		this->sendWorld_UserLeaveWorld(__TcpCenter, user);//1、通知中心服务器 xx玩家掉线
		this->sendWorld_UserLeaveWorld(user->tcpGame, user);//2、通知游戏服务器 xx玩家掉线 
		//this->sendWorld_UserLeaveWorld(__TcpChat, user);//3、通知聊天服务器 xx玩家掉线
		//this->sendWorld_UserLeaveWorld(__TcpTeam, user);//4、通知组队服务器 xx玩家掉线
	}

	//还没有登录 只通知中心服务器xx 连接掉线
	void UserManager::sendWorld_UserLeaveWorld(s64 memid)
	{
		__TcpCenter->begin(CMD_600);
		__TcpCenter->sss(-1);
		__TcpCenter->sss(memid);
		__TcpCenter->end();
	}

	//发送给世界服务器[chat、team]-玩家离开
	void UserManager::sendWorld_UserLeaveWorld(net::ITCPClient* tcp, S_USER_GATE* user)
	{
		if (tcp == NULL) return;
		tcp->begin(CMD_600);
		tcp->sss(user->userindex);
		tcp->sss(user->memid);
		tcp->end();
	}

}

