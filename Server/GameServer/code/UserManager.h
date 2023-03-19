#ifndef  ____USERMANAGER_H
#define  ____USERMANAGER_H

#include <map>
#include "WorldData.h"
#include "IContainer.h"
#include <queue>
#include "CommandData.h"
#include <concurrent_queue.h>


namespace app
{
	class UserManager
	{
	public:
		s32  __OnlineCount;//在线人数
		std::queue<S_LOAD_ROLE*> __UserRequests;				// 请求数据队列
		std::queue<S_LOAD_ROLE*> __UserRequestsPool;			// 请求队列回收池

		S_USER_BASE**			 __OnLineUsers;					// 在线用户数据，指针数组
		std::queue<S_USER_BASE*> __UsersPool;					// 用户对象回收池

		Concurrency::concurrent_queue<u8> __TcpClientDisconnets;//掉线队列

	public:
		UserManager();
		~UserManager();

		void init();
		void update();

		void pushUser(S_USER_BASE* data);
		S_USER_BASE* popUser();
		void clearUser(const u32 index);						// 玩家离开
		bool insertUser(const u32 index, S_USER_BASE* user);	// 新增玩家

		S_USER_BASE* findUser(const u32 index);
		S_USER_BASE* findUser(const u32 index, const u64 memid);
		S_USER_BASE* findUser(const u32 index, const s32 layer);
		S_USER_BASE* findUser(S_RECT_BASE* edge, const u32 index, const s32 layer, const u32 mapid);
		void userDisconnect(S_USER_BASE* user);

		// 请求角色数据回收池
		void pushRequestPool(S_LOAD_ROLE* data);
		S_LOAD_ROLE* popRequestPool();

		// 插入请求开始游戏加载角色数据队列
		void insertRequest(S_LOAD_ROLE* data);
		void onQueueLoadRole();
		// 切换进新的gameserver时向DB要玩家数据
		void onPushLoadRole(net::S_CLIENT_BASE* c, void* d, s32 userindex, u32 mapid, u16 cmd, u16 childcmd);

		inline void setOnlineCount(bool islogin)
		{
			if (islogin) __OnlineCount++;
			else __OnlineCount--;
		}

		inline void setTcpClientDisconnect(u8 type)
		{
			__TcpClientDisconnets.push(type);
		}
	};

	extern UserManager* __UserManager;
}

#endif