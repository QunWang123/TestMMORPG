#include "UserManager.h"
#include "AppGlobal.h"
#include "WorldBC.h"
#include "AppTeam.h"


namespace app
{
	UserManager* __UserManager = nullptr;
	s32 temp_CheckPos = 0;

	UserManager::UserManager()
	{
		init();
	}

	UserManager::~UserManager()
	{

	}

	void UserManager::init()
	{
		__OnlineCount = 0;
		int count = func::__ServerInfo->MaxUser;
		__OnLineUsers = new S_USER_BASE*[count];
		for (int i = 0; i < count; i++)
		{
			__OnLineUsers[i] = NULL;
		}
	}

	void onCheckUser()
	{
		if (temp_CheckPos >= func::__ServerInfo->MaxUser) temp_CheckPos = 0;
		s32  num = temp_CheckPos + 1000;
		if (num > func::__ServerInfo->MaxUser)  num = func::__ServerInfo->MaxUser;

		for (int i = temp_CheckPos; i < num; i++)
		{
			auto user = __UserManager->findUser(i);
			if (user == nullptr ||
				!user->mem.isT()) continue;

			//1、玩家更新
			user->update();
			//2、玩家状态广播
			TS_Broadcast::bc_RoleState(user);

			// heart一定要对齐
			s32 ftime = (int)time(NULL) - user->tmp.temp_HeartTime;
			if (ftime < func::__ServerInfo->HeartTime + 10) continue;

			LOG_MESSAGE("UserDisconnect heart... %d-%lld-%s line:%d\n",
				i, user->mem.id, user->mem.name, __LINE__);
			__UserManager->userDisconnect(user);
		}
		temp_CheckPos += 1000;
	}

	// 加载玩家角色队列
	// 这里为什么要设计成队列 去获取角色数据？
	// 如果有上千玩家同时申请角色数据 在一个峰值内 有可能造成buff缓冲爆满
	void UserManager::onQueueLoadRole()
	{
		if (__TcpDB == nullptr) return;
		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			if (__UserRequests.empty()) return;
			while (!__UserRequests.empty())
			{
				S_LOAD_ROLE* data = __UserRequests.front();
				__UserRequests.pop();
				__TcpServer->begin(data->server_connectid, CMD_500);
				__TcpServer->sss(data->server_connectid, (u16)3099);
				__TcpServer->sss(data->server_connectid, data, sizeof(S_LOAD_ROLE));
				__TcpServer->end(data->server_connectid);
				pushRequestPool(data);
			}
			return;
		}
		if (__UserRequests.empty()) return;
		for (int i = 0; i < MAX_REQUEST_ROLE_COUNT; i++)
		{
			S_LOAD_ROLE* data = __UserRequests.front();
			__UserRequests.pop();

			__TcpDB->begin(data->cmd);
			__TcpDB->sss((u16)0);
			__TcpDB->sss(data, sizeof(S_LOAD_ROLE));
			__TcpDB->end();
			pushRequestPool(data);
			if (__UserRequests.empty()) return;
		}
	}
	void UserManager::onPushLoadRole(net::S_CLIENT_BASE* c, void* d, s32 userindex, u32 mapid, u16 cmd, u16 childcmd)
	{
		if (d == nullptr) return;
		S_COMMAND_GATEBASE* data = (S_COMMAND_GATEBASE*)d;
		if (data == nullptr) return;

		S_LOAD_ROLE* data2 = __UserManager->popRequestPool();
		data2->cmd = cmd;
		data2->user_connectid = data->user_connectid;
		data2->userindex = userindex;
		data2->memid = data->memid;
		data2->mapid = mapid;
		data2->server_connectid = c->ID;
		data2->server_clientid = c->clientID;


		//避免开服或者新开服务器或者切换地图 大量玩家峰值
		//设计为队列加载玩家角色数据到GameServer 
		//把峰值平摊到时间线上 平滑进行
		this->insertRequest(data2);
	}

	//检查和DB连接情况 失去连接通知各个gate关闭连接 玩家已掉线
	void onCheckDisconnect_DB()
	{
		if (__TcpDB == nullptr) return;
		if (__TcpDB->getData()->state >= func::C_CONNECT_SECURE)  return;

		LOG_MESSAGE("disconnect DBServer...\n");

		int count = func::__ServerInfo->MaxUser;
		for (int i = 0; i < count; i++)
		{
			auto user = __UserManager->findUser(i);
			if (user == nullptr)  continue;
			if (!user->mem.isT()) continue;

			//通知网关 玩家掉线 T掉连接在当前gameserver上的所有玩家
			// 即发送一个数据给gataserver，gataserver就closesocket就行
			__TcpServer->begin(user->tmp.server_connectid, CMD_602);
			__TcpServer->sss(user->tmp.server_connectid, user->tmp.userindex);
			__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
			__TcpServer->end(user->tmp.server_connectid);
		}
	}
	//检查和Team连接情况 清理全部玩家的队伍连接
	void onCheckDisconnect_Team()
	{
		if (__TcpCenter == nullptr) return;
		if (__TcpCenter->getData()->state >= func::C_CONNECT_SECURE)  return;

		LOG_MESSAGE("disconnect TeamServer...\n");

		//1、清理全部队伍数据
		if (__AppTeam != nullptr) __AppTeam->reset();
		//2、清理全部玩家队伍数据
		int count = func::__ServerInfo->MaxUser;
		for (int i = 0; i < count; i++)
		{
			auto user = __UserManager->findUser(i);
			if (user == nullptr)  continue;
			if (!user->mem.isT()) continue;

			user->tmp.setTeamIndex(-1, -1);
		}
	}

	void UserManager::update()
	{
		//1、处理当前做为客户端 掉线的情况
		if (!__TcpClientDisconnets.empty())
		{
			int num = 0;
			while (true)
			{
				u8 server_type = 0;
				__TcpClientDisconnets.try_pop(server_type);

				switch (server_type)
				{
				case func::S_TYPE_DB:
					onCheckDisconnect_DB();
					break;
				case func::S_TYPE_CENTER:
					onCheckDisconnect_Team();
					break;
				}
				if (__TcpClientDisconnets.empty()) break;
				//避免死循环
				++num;
				if (num >= 10) break;
			}
		}
		onQueueLoadRole();
		onCheckUser();
	}

	void UserManager::pushUser(S_USER_BASE* data)
	{
		int count = __UsersPool.size();
		if (count > 1000)
		{
			delete data;
		}
		else
		{
			data->reset();
			__UsersPool.push(data);
		}
	}

	S_USER_BASE* UserManager::popUser()
	{
		S_USER_BASE* data = nullptr;
		if (__UsersPool.empty())
		{
			data = new app::S_USER_BASE();
		}
		else
		{
			data = __UsersPool.front();
			__UsersPool.pop();
			data->reset();
		}
		return data;
	}

	// 玩家离开时，只要调用一下pushRequestPool和clearuser就ok
	void UserManager::clearUser(const u32 index)
	{
		int count = func::__ServerInfo->MaxUser;
		if (index >= count) return;
		__OnLineUsers[index] = NULL;
	}

	bool UserManager::insertUser(const u32 index, S_USER_BASE* user)
	{
		int count = func::__ServerInfo->MaxUser;
		if (index >= count) return false;

		__OnLineUsers[index] = user;
		return true;
	}

	S_USER_BASE* UserManager::findUser(const u32 index)
	{
		int count = func::__ServerInfo->MaxUser;
		if (index >= count) return NULL;

		return __OnLineUsers[index];
	}

	S_USER_BASE* UserManager::findUser(const u32 index, const u64 memid)
	{
		S_USER_BASE* user = findUser(index);
		if (user == nullptr || !user->mem.isT_ID(memid)) return nullptr;

		return user;
	}

	app::S_USER_BASE* UserManager::findUser(const u32 index, const s32 layer)
	{
		S_USER_BASE* user = findUser(index);
		if (user == nullptr || !user->mem.isT() || user->node.layer != layer) return nullptr;

		return user;
	}

	app::S_USER_BASE* UserManager::findUser(S_RECT_BASE* edge, const u32 index, const s32 layer, const u32 mapid)
	{
		S_USER_BASE* user = findUser(index, layer);
		if (user == nullptr || 
			user->role.base.status.mapid != mapid ||
			!user->isLive() ||
			!edge->inEdge(&user->bc.grid_big)) return nullptr;
		return user;
	}

	// 玩家掉线
	void UserManager::userDisconnect(S_USER_BASE* user)
	{
		if (__TcpDB->getData()->state >= func::C_CONNECT_SECURE)
		{
			//保存状态相关数据
			TS_Broadcast::db_SaveStatusInfo(user);
			//通知DB掉线
			TS_Broadcast::db_Disconnect(user->tmp.userindex, user->mem.id, user->mem.timeLastLogin);
		}
		__UserManager->setOnlineCount(false);
		// 广播给九宫格中的玩家你已经离开
		TS_Broadcast::bc_RoleLeaveWorld(user);
		this->clearUser(user->tmp.userindex);

		// 设置离开世界
		user->leaveWorld();
		user->reset();
		this->pushUser(user);
	}

	void UserManager::pushRequestPool(S_LOAD_ROLE* data)
	{
		int size = __UserRequestsPool.size();
		if (size > 1000)
		{
			delete data;
		}
		else
		{
			data->reset();
			__UserRequestsPool.push(data);
		}
	}

	S_LOAD_ROLE* UserManager::popRequestPool()
	{
		S_LOAD_ROLE* role = nullptr;
		if (__UserRequestsPool.empty())
		{
			role = new S_LOAD_ROLE();
		}
		else
		{
			role = __UserRequestsPool.front();
			__UserRequestsPool.pop();
			role->reset();
		}
		return role;
	}

	void UserManager::insertRequest(S_LOAD_ROLE* data)
	{
		__UserRequests.push(data);
	}
}
