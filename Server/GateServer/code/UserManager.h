#ifndef  ____USERMANAGER_H
#define  ____USERMANAGER_H

#include <map>
#include "WorldData.h"
#include "IContainer.h"
#include <concurrent_queue.h>

namespace app
{
	class UserManager
	{
	public:
		HashArray<S_USER_GATE>* __onlineUsers;

		Concurrency::concurrent_queue<u8>  __TcpClientDisconnets;//掉线队列
	public:
		UserManager();
		~UserManager();

		void Init();
		void update();//更新

		S_USER_GATE* findUser(const u32 connectid);
		S_USER_GATE* findUser(const u32 connectid, const u64 memid);
		S_USER_GATE* findUser_Connection(const u32 connectid, const u64 memid);

		net::ITCPClient* findGameServer_Connection(u8 line, u32 mapid);

		void sendWorld_UserLeaveWorld(s64 memid);
		void sendWorld_UserLeaveWorld(S_USER_GATE* user);
		void sendWorld_UserLeaveWorld(net::ITCPClient* tcp, S_USER_GATE* user);

		void sendWorld_UserEntryWorld(net::ITCPClient* tcp, S_USER_GATE* user);
		void sendWorld_UserEntryWorld(net::ITCPClient* tcp);
		void sendWorld_UserEntryWorld(S_USER_GATE* user);
		void sendWorld_UserEntryWorld();

		inline void setTcpClientDisconnect(u8 type)
		{
			__TcpClientDisconnets.push(type);
		}
	};

	extern UserManager* __UserManager;
}

#endif