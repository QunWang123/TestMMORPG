#include "UserManager.h"


namespace app
{
	UserManager* __UserManager = nullptr;
	char checkname[USER_MAX_MEMBER_CHECK];

	UserManager::UserManager()
	{

	}

	UserManager::~UserManager()
	{

	}

	void UserManager::insetAccount(S_ACCOUNT_BASE* acc)
	{
		memset(checkname, 0, USER_MAX_MEMBER_CHECK);
		memcpy(checkname, acc->name, USER_MAX_MEMBER);
		for (int i = 0; i < 20; i++)
		{
			checkname[i] = tolower(checkname[i]);
		}
		__Accounts.insert(std::make_pair(checkname, acc));
	}

	S_ACCOUNT_BASE* UserManager::findAccount(char* name)
	{
		memset(checkname, 0, USER_MAX_MEMBER_CHECK);
		memcpy(checkname, name, USER_MAX_MEMBER);
		for (int i = 0; i < 20; i++)
		{
			checkname[i] = tolower(checkname[i]);
		}

		auto it = __Accounts.find(checkname);
		if (it != __Accounts.end())
		{
			auto account = it->second;
			return account;
		}
		return nullptr;
	}

}

