#ifndef  ____USERMANAGER_H
#define  ____USERMANAGER_H

#include <map>
#include "WorldData.h"
#include "IContainer.h"


namespace app
{
	class UserManager
	{
	public:
		// 2. 游戏账号
		std::map<u64, S_USER_MEMBER_BASE*>			__MemberIDs;		// 游戏账号快表，ID为键
		std::map<std::string, S_USER_MEMBER_BASE*>	__MemberNames;		// 游戏账号快表，名称为键
		// 3. 昵称
		std::map<std::string, u8>					__Nicks;	// 全部昵称块表，昵称为键，防止昵称重复
		// 4.在线状态玩家块表
		std::map<u64, S_USER_MEMBER_BASE*>			__MemberStates;		// 游戏账号，遍历快表
		// 5. 最大在线玩家，hash快表
		HashArray<S_USER_ONLINE>*					__OnLineUsers;		// 在线玩家数据

	public:
		UserManager();
		~UserManager();

		void Init();
		void updata(); 
		// 掉线
		void userDisconnect(s32 userindex, s64 memid, u16 childcmd);
		// 保存基础数据
		void userBaseData(s32 userindex, s64 memid);

		// 游戏账号
		void insertMember(S_USER_MEMBER_BASE* m);
		S_USER_MEMBER_BASE* findMember(u64 memid);
		S_USER_MEMBER_BASE* findMember(char* name);
	
		void insertMemberStates(S_USER_MEMBER_BASE* m);

		// 角色昵称
		void insertNick(char* nick);
		bool findNick(char* nick);
		void deleteNick(char* nick);
		// 在线玩家数据
		S_USER_ONLINE* findUser(const u32 index, const u64 memid);
		S_USER_ONLINE* findUser(const u32 index);
		S_USER_ONLINE* findFreeUser();

		void updateMemberMinrole(S_USER_ONLINE* user);//更新账号下面的MIN角色数据 只在掉线情况下更新
	};

	extern UserManager* __UserManager;
}

#endif