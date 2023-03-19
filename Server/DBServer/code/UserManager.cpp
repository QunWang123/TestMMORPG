#include "UserManager.h"
#include "IDefine.h"
#include "CommandData.h"
#include <time.h>
#include "DBManager.h"

namespace app
{
	UserManager* __UserManager = nullptr;
	char checkname[USER_MAX_MEMBER_CHECK];
	int temp_checktime = 0;


	UserManager::UserManager()
	{
		__OnLineUsers = nullptr;
		Init();
	}

	UserManager::~UserManager()
	{

	}

	void UserManager::Init()
	{
		int count = func::__ServerInfo->MaxUser;
		__OnLineUsers = new HashArray<S_USER_ONLINE>(count);
		for (int i = 0; i < count; i++)
		{
			S_USER_ONLINE* user = __OnLineUsers->Value(i);
			user->reset();
		}
	}
	
	// 玩家掉线，推送给数据库保存数据
	void UserManager::userDisconnect(s32 userindex, s64 memid, u16 childcmd)
	{
		auto db = __DBManager->GetDBSource(ETT_USERWRITE);
		auto buff = db->PopBuffer();
		buff->b(CMD_600);
		buff->s(childcmd);
		buff->s(userindex);
		buff->s(memid);
		buff->e();
		db->PushToThread(buff);
	}

	void UserManager::userBaseData(s32 userindex, s64 memid)
	{
		auto db = __DBManager->GetDBSource(ETT_USERWRITE);
		auto buff = db->PopBuffer();
		buff->b(CMD_610);
		buff->s(userindex);
		buff->s(memid);
		buff->e();
		db->PushToThread(buff);
	}

	void onCheckHeart()
	{
		int ftime = (int)time(NULL) - temp_checktime;
		if (ftime < 2) return;
		temp_checktime = (int)time(NULL);

		// 遍历在线玩家
		auto it = __UserManager->__MemberStates.begin();
		while (it != __UserManager->__MemberStates.end())
		{
			auto m = it->second;
			if (m->mem.state == M_FREE)
			{
				it = __UserManager->__MemberStates.erase(it);
				continue;
			}
			// 选角色状态
			if (m->mem.state < M_SELECTROLE)
			{
				++it;
				continue;
			}
			// 未登录状态
			if (m->mem.state < M_LOGINEND)
			{
				ftime = (int)time(NULL) - m->tmp.temp_HeartTime;
				if (ftime >= func::__ServerInfo->HeartTime + 30)
				{
					m->mem.state = M_FREE;
					LOG_MESSAGE("mem disconnect heart %lld line:%d\n", m->mem.id, __LINE__);
				}
				++it;
				continue;
			}

			// 在线状态
			auto user = __UserManager->findUser(m->tmp.userindex);
			if (user == nullptr || 
				user->mem.isT() || 
				user->mem.state < M_LOGINEND || 
				user->mem.state == M_SAVING)
			{
				++it;
				continue;
			}

			// 掉线检测， 需要保存数据
			ftime = (int)time(NULL) - m->tmp.temp_HeartTime;
			if (ftime >= func::__ServerInfo->HeartTime + 30)
			{
				LOG_MESSAGE("user disconnect heart %lld line:%d\n", m->mem.id, __LINE__);
				m->mem.state = M_SAVING;
				user->mem.state = M_SAVING;
				__UserManager->userDisconnect(user->tmp.userindex, user->mem.id, 100);
				++it;
				continue;
			}
			
			// 5分钟保存一次
			ftime = (int)time(NULL) - user->tmp.temp_SaveTime;
			if (ftime > 300)
			{
				LOG_MESSAGE("user auto save %lld line:%d\n", user->mem.id, __LINE__);
				user->tmp.temp_SaveTime = (int)time(NULL);
				__UserManager->userDisconnect(user->tmp.userindex, user->mem.id, 0);
			}

			++it;
		}
	}

	// 玩家数据更新
	void UserManager::updata()
	{
		// 1. 检查心跳包
		onCheckHeart();

	}

	void UserManager::insertMember(S_USER_MEMBER_BASE* m)
	{
		memset(checkname, 0, USER_MAX_MEMBER_CHECK);
		memcpy(checkname, m->mem.name, USER_MAX_MEMBER);
		for (int i = 0; i < 20; i++)
		{
			checkname[i] = tolower(checkname[i]);
		}
		__MemberNames.insert(std::make_pair(checkname, m));
		__MemberIDs.insert(std::make_pair(m->mem.id, m));
	}

	app::S_USER_MEMBER_BASE* UserManager::findMember(u64 memid)
	{
		auto it = __MemberIDs.find(memid);
		if (it != __MemberIDs.end())
		{
			auto mem = it->second;
			return mem;
		}
		return nullptr;
	}

	app::S_USER_MEMBER_BASE* UserManager::findMember(char* name)
	{
		memset(checkname, 0, USER_MAX_MEMBER_CHECK);
		memcpy(checkname, name, USER_MAX_MEMBER);
		for (int i = 0; i < 20; i++)
		{
			checkname[i] = tolower(checkname[i]);
		}
		auto it = __MemberNames.find(checkname);
		if (it != __MemberNames.end())
		{
			auto mem = it->second;
			return mem;
		}
		return nullptr;
	}

	void UserManager::insertMemberStates(S_USER_MEMBER_BASE* m)
	{
		if (m == NULL) return;
		__MemberStates.insert(std::make_pair(m->mem.id, m));
	}

	void UserManager::insertNick(char* nick)
	{
		if (nick == NULL || strlen(nick) < 1) return;
		__Nicks.insert(std::make_pair(nick, 1));
	}

	bool UserManager::findNick(char* nick)
	{
		if (nick == NULL || strlen(nick) < 1) return false;
		auto it = __Nicks.find(nick);
		if (it != __Nicks.end())
		{
			return true;
		}
		return false;
	}
	
	void UserManager::deleteNick(char* nick)
	{
		auto it = __Nicks.find(nick);
		if (it != __Nicks.end())
		{
			__Nicks.erase(it);
		}
	}

	app::S_USER_ONLINE* UserManager::findUser(const u32 index)
	{
		if (index >= __OnLineUsers->length) return nullptr;

		return __OnLineUsers->Value(index);
	}
	app::S_USER_ONLINE* UserManager::findUser(const u32 index, const u64 memid)
	{
		auto user = findUser(index);
		if (user == nullptr || memid != user->mem.id) return nullptr;
		
		return user;
	}


	app::S_USER_ONLINE* UserManager::findFreeUser()
	{
		int count = __OnLineUsers->length;
		for (int i = 0; i < count; i++)
		{
			auto user = __OnLineUsers->Value(i);
			if (user == nullptr) continue;
			if (user->mem.isT())continue;

			user->reset();
			user->tmp.userindex = i;
			return user;
		}
		return nullptr;
	}

	//更新保存 账号下面的角色mini数据
	void UserManager::updateMemberMinrole(S_USER_ONLINE* user)
	{
		auto m = __UserManager->findMember(user->mem.id);
		if (m == NULL)
		{
			LOG_MESSAGE("updateMemberMinrole m=null %lld  line:%d\n", user->mem.id, __LINE__);
			return;
		}
		if (m->mem.state != M_SAVING)
		{
			LOG_MESSAGE("updateMemberMinrole state err= %lld  line:%d\n", user->mem.id, __LINE__);
			return;
		}
		if (m->tmp.roleindex >= USER_MAX_ROLE)
		{
			LOG_MESSAGE("updateMemberMinrole state err= %lld  line:%d\n", user->mem.id, __LINE__);
			return;
		}

		//更新下 
		S_USER_MEMBER_ROLE* r = &m->role[m->tmp.roleindex];
		r->level = user->role.base.exp.level;
		r->clothid = 0;
		r->weaponid = 0;
		memcpy(r->nick, user->role.base.innate.nick, USER_MAX_NICK);

		auto db = __DBManager->GetDBSource(ETT_USERWRITE);
		auto buff = db->PopBuffer();
		buff->b(CMD_601);
		buff->s((s64)user->mem.id);
		buff->s(m->tmp.roleindex);
		buff->s(r, sizeof(S_USER_MEMBER_ROLE));
		buff->e();
		db->PushToThread(buff);
	}

}

