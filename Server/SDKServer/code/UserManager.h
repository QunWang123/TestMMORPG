#ifndef  ____USERMANAGER_H
#define  ____USERMANAGER_H

#include <map>
#include "WorldData.h"


namespace app
{
	class UserManager
	{
	public:
		// 1.运维账号
		std::map<std::string, S_ACCOUNT_BASE*> __Accounts;	// 如果是自己的运维，必须有，第三方维护没有这个表
	public:
		UserManager();
		~UserManager();

		void insetAccount(S_ACCOUNT_BASE* acc);
		S_ACCOUNT_BASE* findAccount(char* name);
	};

	extern UserManager* __UserManager;
}

#endif