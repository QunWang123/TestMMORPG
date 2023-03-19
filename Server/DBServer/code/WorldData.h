#ifndef  ____WORLDDATA_H
#define  ____WORLDDATA_H

#include <cstring>
#include <map>
#include "UserData.h"
#include "INetBase.h"

namespace app
{

#pragma pack(push,packing)
#pragma pack(1)
	

	// �����������
	struct S_USER_ONLINE 
	{
		this_constructor(S_USER_ONLINE);

		S_USER_MEMBER_DATA		mem;			// �˺Ż�������
		S_USER_ROLE				role;			// ��������
		S_USER_TEMP_BASE		tmp;			// ��ʱ����

		inline void reset()
		{
			memset(this, 0, sizeof(S_USER_ONLINE));
			tmp.reset();
		}
	};

#pragma pack(pop, packing)
	inline void sendErrInfo(net::ITCPServer* ts, s32 connectid, u16 cmd, u16 childcmd, void* data, u16 size)
	{
		ts->begin(connectid, cmd);
		ts->sss(connectid, childcmd);
		ts->sss(connectid, data, size);
		ts->end(connectid);
	}
}
#endif