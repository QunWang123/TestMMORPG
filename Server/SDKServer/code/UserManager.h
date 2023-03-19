#ifndef  ____USERMANAGER_H
#define  ____USERMANAGER_H

#include <map>
#include "WorldData.h"


namespace app
{
	class UserManager
	{
	public:
		// 1.��ά�˺�
		std::map<std::string, S_ACCOUNT_BASE*> __Accounts;	// ������Լ�����ά�������У�������ά��û�������
	public:
		UserManager();
		~UserManager();

		void insetAccount(S_ACCOUNT_BASE* acc);
		S_ACCOUNT_BASE* findAccount(char* name);
	};

	extern UserManager* __UserManager;
}

#endif