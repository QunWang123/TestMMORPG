#ifndef __WORLDDATA_H
#define __WORLDDATA_H
#include "INetBase.h"
#include "UserData.h"

namespace app
{
#pragma pack(push, packing)
#pragma pack(1)
	// �����ϵ��û�����
	struct S_USER_GATE
	{
		u8			state;							// ��ǰ״̬
		u8			line;							// ��ǰ������·
		u64			memid;							// ��ǰ��¼����Ϸ�˺�
		u32			mapid;							// ��ǰ���ڵ�ͼid
		s32			userindex;						// ��Ϸ�������Ϲ�ϣ���������
		u16			level;
		u32			temp_RoleTime;					// �����¼��Ϸ��ȡ��ɫ����ʱ��
		u32			temp_MoveSpeed;					// �ƶ��ٶ�
		u32			temp_GetCompatTime;				// ��ȡս��װ��ʱ�� 
		u32			temp_GetBagTime;				// ��ȡ����ʱ�� 
// 		u32			temp_GetWareHouseTime;			// ��ȡ�ֿ�ʱ�� 
// 		u32			temp_GetTaskTime;				// ��ȡ����ʱ�� 
		u32			temp_GetChangeMapTime;			// ��ȡ�л���ͼʱ��
		u32			temp_GetCopyTime;				// ��ȡ��������ʱ��
		char		name[USER_MAX_MEMBER];
		char		nick[USER_MAX_NICK];

		net::ITCPClient*		tcpGame;			// ������Ϸ��������ͨ��ָ��
		net::S_CLIENT_BASE*		connection;			// ��¼�������������(����engine����Ǹ��ͻ���)

		inline void reset()
		{
			state = 0;
			line = 0;
			memid = 0;
			mapid = 0;
			userindex = -1;
			temp_RoleTime = 0;
			temp_MoveSpeed = 0;
			temp_GetCompatTime = 0;
			temp_GetBagTime = 0;
			// temp_GetWareHouseTime = 0;
			// temp_GetTaskTime = 0;
			temp_GetChangeMapTime = 0;
			temp_GetCopyTime = 0;
			memset(name, 0, USER_MAX_MEMBER);
			memset(nick, 0, USER_MAX_NICK);

			tcpGame = nullptr;
			connection = nullptr;
		}
	};
#pragma pack(pop, packing)

}



#endif // __WORLDDATA_H
