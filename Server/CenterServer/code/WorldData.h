#ifndef __WORLDDATA_H
#define __WORLDDATA_H

#include "INetBase.h"
#include "UserData.h"
#include "AppGlobal.h"
#include "ShareFunction.h"

namespace app
{
	

#pragma pack(push, packing)
#pragma pack(1)

	struct S_ACCOUNT_KEY
	{
		this_constructor(S_ACCOUNT_KEY);

		char name[USER_MAX_MEMBER];		// �˺�
		char key[USER_MAX_KEY];			// ��Կ
		u32 keytime;					// ��Կ����ʱ��

		inline bool isT_Key(const char* value) { return(strcmp(key, value) == 0); };

	};

	struct S_USER_TEAM
	{
		this_constructor(S_USER_TEAM);

		S_USER_MEMBER_DATA		mem;			// �˺Ż�����Ϣ
		S_USER_MEMBER_ROLE		role;			// ��ɫ����
		S_ROLE_BASE_STATUS		status;			// ״̬��Ϣ
		S_USER_TEMP_BASE		tmp;			// ��ʱ����

		inline void reset()
		{
			memset(this, 0, sizeof(S_USER_TEAM));
			tmp.reset();
		}
	};
	//���ʹ�������
	inline void sendErrInfo(s32 connectid, u16 cmd, u16 childcmd, void* data, u32 size)
	{
		__TcpServer->begin(connectid, cmd);
		__TcpServer->sss(connectid, childcmd);
		__TcpServer->sss(connectid, data, size);
		__TcpServer->end(connectid);
	}
	//����line ��ͼIDѰ��һ����Ϸ����������
	inline net::S_CLIENT_BASE* findGameServer_Connection(const u8 line, const u32 mapid)
	{
		u32  gameserverid = share::findGameServerID(line, mapid);
		net::S_CLIENT_BASE* c = __TcpServer->client(MAX_SERVER_ID, gameserverid);
		return c;
	}
#pragma pack(pop, packing)

}



#endif // __WORLDDATA_H
