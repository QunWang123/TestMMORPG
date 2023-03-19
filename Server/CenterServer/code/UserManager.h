#ifndef  ____USERMANAGER_H
#define  ____USERMANAGER_H

#include <map>
#include "WorldData.h"
#include "IContainer.h"
#include <queue>
#include "CommandData.h"
namespace app
{
	class UserManager
	{
	public:
		s32  __OnlineCount;
		HashArray<S_USER_TEAM>*					__onlineUsers;
		// 1. 账号-密钥快表
		std::map<std::string, S_ACCOUNT_KEY*>		__AccountsKey;//账号秘钥表
		//二、登录请求队列
		bool __IsRequesting;//请求开关 当前是否正在请求数据
		s64  __RequestTime;  //请求时间
		
		std::queue<S_SELECT_ROLE*>					__UserRequests;				// 请求数据队列
		std::queue<S_SELECT_ROLE*>					__UserRequestsPool;			// 请求队列回收池

	public:
		UserManager();
		~UserManager();

		void Init();
		void update();
		void onQueueSelectRole();

		S_USER_TEAM* findUser(const u32 connectid);
		S_USER_TEAM* findUser(const u32 connectid, const u64 memid);

		// 密钥账号
		void insetAccount(S_ACCOUNT_KEY* acc);			// 插入账号块表
		S_ACCOUNT_KEY* findAccountKey(char* name);			// 查找账号快表

		// 请求角色数据回收池
		void pushRequestPool(S_SELECT_ROLE* data);
		S_SELECT_ROLE* popRequestPool();

		// 插入请求开始游戏加载角色数据队列
		void insertRequest(S_SELECT_ROLE* data);

		inline void setIsRequesting(bool value) { __IsRequesting = value; }
		inline s32  getRequestCount() { return __UserRequests.size(); }

		void setUserDisconnect(s32 serverid);
	};

	extern UserManager* __UserManager;
}

#endif