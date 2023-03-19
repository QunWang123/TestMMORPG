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
		// 1. �˺�-��Կ���
		std::map<std::string, S_ACCOUNT_KEY*>		__AccountsKey;//�˺���Կ��
		//������¼�������
		bool __IsRequesting;//���󿪹� ��ǰ�Ƿ�������������
		s64  __RequestTime;  //����ʱ��
		
		std::queue<S_SELECT_ROLE*>					__UserRequests;				// �������ݶ���
		std::queue<S_SELECT_ROLE*>					__UserRequestsPool;			// ������л��ճ�

	public:
		UserManager();
		~UserManager();

		void Init();
		void update();
		void onQueueSelectRole();

		S_USER_TEAM* findUser(const u32 connectid);
		S_USER_TEAM* findUser(const u32 connectid, const u64 memid);

		// ��Կ�˺�
		void insetAccount(S_ACCOUNT_KEY* acc);			// �����˺ſ��
		S_ACCOUNT_KEY* findAccountKey(char* name);			// �����˺ſ��

		// �����ɫ���ݻ��ճ�
		void pushRequestPool(S_SELECT_ROLE* data);
		S_SELECT_ROLE* popRequestPool();

		// ��������ʼ��Ϸ���ؽ�ɫ���ݶ���
		void insertRequest(S_SELECT_ROLE* data);

		inline void setIsRequesting(bool value) { __IsRequesting = value; }
		inline s32  getRequestCount() { return __UserRequests.size(); }

		void setUserDisconnect(s32 serverid);
	};

	extern UserManager* __UserManager;
}

#endif