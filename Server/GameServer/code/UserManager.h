#ifndef  ____USERMANAGER_H
#define  ____USERMANAGER_H

#include <map>
#include "WorldData.h"
#include "IContainer.h"
#include <queue>
#include "CommandData.h"
#include <concurrent_queue.h>


namespace app
{
	class UserManager
	{
	public:
		s32  __OnlineCount;//��������
		std::queue<S_LOAD_ROLE*> __UserRequests;				// �������ݶ���
		std::queue<S_LOAD_ROLE*> __UserRequestsPool;			// ������л��ճ�

		S_USER_BASE**			 __OnLineUsers;					// �����û����ݣ�ָ������
		std::queue<S_USER_BASE*> __UsersPool;					// �û�������ճ�

		Concurrency::concurrent_queue<u8> __TcpClientDisconnets;//���߶���

	public:
		UserManager();
		~UserManager();

		void init();
		void update();

		void pushUser(S_USER_BASE* data);
		S_USER_BASE* popUser();
		void clearUser(const u32 index);						// ����뿪
		bool insertUser(const u32 index, S_USER_BASE* user);	// �������

		S_USER_BASE* findUser(const u32 index);
		S_USER_BASE* findUser(const u32 index, const u64 memid);
		S_USER_BASE* findUser(const u32 index, const s32 layer);
		S_USER_BASE* findUser(S_RECT_BASE* edge, const u32 index, const s32 layer, const u32 mapid);
		void userDisconnect(S_USER_BASE* user);

		// �����ɫ���ݻ��ճ�
		void pushRequestPool(S_LOAD_ROLE* data);
		S_LOAD_ROLE* popRequestPool();

		// ��������ʼ��Ϸ���ؽ�ɫ���ݶ���
		void insertRequest(S_LOAD_ROLE* data);
		void onQueueLoadRole();
		// �л����µ�gameserverʱ��DBҪ�������
		void onPushLoadRole(net::S_CLIENT_BASE* c, void* d, s32 userindex, u32 mapid, u16 cmd, u16 childcmd);

		inline void setOnlineCount(bool islogin)
		{
			if (islogin) __OnlineCount++;
			else __OnlineCount--;
		}

		inline void setTcpClientDisconnect(u8 type)
		{
			__TcpClientDisconnets.push(type);
		}
	};

	extern UserManager* __UserManager;
}

#endif