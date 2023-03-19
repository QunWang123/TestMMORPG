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
		// 2. ��Ϸ�˺�
		std::map<u64, S_USER_MEMBER_BASE*>			__MemberIDs;		// ��Ϸ�˺ſ��IDΪ��
		std::map<std::string, S_USER_MEMBER_BASE*>	__MemberNames;		// ��Ϸ�˺ſ������Ϊ��
		// 3. �ǳ�
		std::map<std::string, u8>					__Nicks;	// ȫ���ǳƿ���ǳ�Ϊ������ֹ�ǳ��ظ�
		// 4.����״̬��ҿ��
		std::map<u64, S_USER_MEMBER_BASE*>			__MemberStates;		// ��Ϸ�˺ţ��������
		// 5. ���������ң�hash���
		HashArray<S_USER_ONLINE>*					__OnLineUsers;		// �����������

	public:
		UserManager();
		~UserManager();

		void Init();
		void updata(); 
		// ����
		void userDisconnect(s32 userindex, s64 memid, u16 childcmd);
		// �����������
		void userBaseData(s32 userindex, s64 memid);

		// ��Ϸ�˺�
		void insertMember(S_USER_MEMBER_BASE* m);
		S_USER_MEMBER_BASE* findMember(u64 memid);
		S_USER_MEMBER_BASE* findMember(char* name);
	
		void insertMemberStates(S_USER_MEMBER_BASE* m);

		// ��ɫ�ǳ�
		void insertNick(char* nick);
		bool findNick(char* nick);
		void deleteNick(char* nick);
		// �����������
		S_USER_ONLINE* findUser(const u32 index, const u64 memid);
		S_USER_ONLINE* findUser(const u32 index);
		S_USER_ONLINE* findFreeUser();

		void updateMemberMinrole(S_USER_ONLINE* user);//�����˺������MIN��ɫ���� ֻ�ڵ�������¸���
	};

	extern UserManager* __UserManager;
}

#endif