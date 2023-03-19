#include "UserManager.h"
#include "AppGlobal.h"
#include "ShareFunction.h"
#include "AppTeam.h"


namespace app
{
	UserManager* __UserManager = nullptr;
	int temp_time_user = 0;
	char checkname[USER_MAX_MEMBER_CHECK];

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
		__RequestTime = 0;
		__IsRequesting = false;
		__OnlineCount = 0;
		// ���������û��ģ������Ϸ��������
		
		// DB��center�е�MaxUser����һ�£���
		__onlineUsers = new HashArray<S_USER_TEAM>(func::__ServerInfo->MaxUser);
		for (int i = 0; i < __onlineUsers->length; i++)
		{
			S_USER_TEAM* user = __onlineUsers->Value(i);
			user->reset();
		}
	}

	void onCheckHeart()
	{
		int ftime = (int)time(NULL) - temp_time_user;
		if (ftime < 2)
		{
			return;
		}
		temp_time_user = (int)time(NULL);
		
		int count = func::__ServerInfo->MaxUser;
		for (int i = 0; i < count; i++)
		{
			auto user = __UserManager->findUser(i);
			if (user == nullptr ||
				!user->mem.isT() ||
				user->mem.state < M_LOGINEND) continue;

			// heartһ��Ҫ����
			ftime = (int)time(NULL) - user->tmp.temp_HeartTime;
			if(ftime < func::__ServerInfo->HeartTime + 10) continue;

			LOG_MESSAGE("onUserDisconnect heart... %d-%lld-%s line:%d\n", 
				i, user->mem.id, user->mem.name, __LINE__);
			user->reset();
		}
	}

	// ���У����Ϳ�ʼ��Ϸ�������ݶ���
	void UserManager::onQueueSelectRole()
	{
		if (__TcpDB == nullptr) return;
		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			if (__UserRequests.empty()) return;
			while (!__UserRequests.empty())
			{
				S_SELECT_ROLE* role = __UserRequests.front();
				__UserRequests.pop();
				__TcpServer->begin(role->server_connectid, CMD_200);
				// __TcpServer->sss(role->server_connectid, (u16)2099);
				__TcpServer->sss(role->server_connectid, role, sizeof(S_SELECT_ROLE));
				__TcpServer->end(role->server_connectid);
				pushRequestPool(role);
			}
			return;
		}
		if (__UserRequests.empty()) return;

		//��ǰ������������ ���30���� ������ػ���������״̬ ������Ϊflase 
		if (__IsRequesting)
		{
			s32 value = time(NULL) - __RequestTime;
			if (value < 10) return;
		}
		__IsRequesting = true;
		__RequestTime = time(NULL);

		S_SELECT_ROLE* role = __UserRequests.front();
		__UserRequests.pop();

		__TcpDB->begin(CMD_200);
		__TcpDB->sss(role, sizeof(S_SELECT_ROLE));
		__TcpDB->end();
		pushRequestPool(role);
	}

	void UserManager::update()
	{
		onCheckHeart();
		onQueueSelectRole();
	}

	// ������Կ���
	void UserManager::insetAccount(S_ACCOUNT_KEY* acc)
	{
		memset(checkname, 0, USER_MAX_MEMBER_CHECK);
		memcpy(checkname, acc->name, USER_MAX_MEMBER);
		for (int i = 0; i < 20; i++)
		{
			checkname[i] = tolower(checkname[i]);
		}
		__AccountsKey.insert(std::make_pair(checkname, acc));
	}

	// ������Կ�˺�
	S_ACCOUNT_KEY* UserManager::findAccountKey(char* name)
	{
		memset(checkname, 0, USER_MAX_MEMBER_CHECK);
		memcpy(checkname, name, USER_MAX_MEMBER);
		for (int i = 0; i < 20; i++)
		{
			checkname[i] = tolower(checkname[i]);
		}

		auto it = __AccountsKey.find(checkname);
		if (it != __AccountsKey.end())
		{
			auto account = it->second;
			return account;
		}
		return nullptr;
	}

	
	// �������
	app::S_USER_TEAM* UserManager::findUser(const u32 connectid)
	{
		if (connectid >= __onlineUsers->length) return nullptr;
		return __onlineUsers->Value(connectid);
	}

	app::S_USER_TEAM* UserManager::findUser(const u32 connectid, const u64 memid)
	{
		auto user = findUser(connectid);
		if (user == nullptr || !user->mem.isT() || user->mem.id != memid) return nullptr;
		return user;
	}

	void UserManager::pushRequestPool(S_SELECT_ROLE* data)
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

	S_SELECT_ROLE* UserManager::popRequestPool()
	{
		S_SELECT_ROLE* role = nullptr;
		if (__UserRequestsPool.empty())
		{
			role = new S_SELECT_ROLE();
		}
		else
		{
			role = __UserRequestsPool.front();
			__UserRequestsPool.pop();
			role->reset();
		}
		return role;
	}

	void UserManager::insertRequest(S_SELECT_ROLE* data)
	{
		__UserRequests.push(data);
	}

	void UserManager::setUserDisconnect(s32 serverid)
	{
		u32 type = share::serverIDToType(serverid);

		for (int i = 0; i < __onlineUsers->length; i++)
		{
			S_USER_TEAM* user = __onlineUsers->Value(i);
			if (user == nullptr) continue;
			if (user->mem.isT() == false) continue;
			if (user->tmp.server_clientid != serverid) continue;

			if (type == func::S_TYPE_GATE) __AppTeam->setTeamLeave(user, EDT_DISCONNET_GATE);
			else  __AppTeam->setTeamLeave(user, EDT_DISCONNET_GAME);

			user->reset();

			this->__OnlineCount--;
		}
	}
}

