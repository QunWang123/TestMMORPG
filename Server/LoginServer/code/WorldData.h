#ifndef  ____WORLDDATA_H
#define  ____WORLDDATA_H

#include "IDefine.h"
#include <string>
#include <cstring>
#include <map>
#include "UserData.h"

namespace app
{

#pragma pack(push,packing)
#pragma pack(1)

	// 1.ткн╛ук╨е
	struct S_ACCOUNT_BASE
	{
		char name[USER_MAX_MEMBER];
		char password[USER_MAX_PASS];
		inline bool isT_Name(const char* value) { 
#ifdef ___WIN32_
			return stricmp(name, value) == 0;
#else
			return stricasecmp(name, value) == 0;
#endif 
		};
		inline bool isT_Pass(const char* value) { return stricmp(password, value) == 0; };
		inline void reset() { memset(this, 0, sizeof(S_ACCOUNT_BASE)); };
	};

	struct S_LOGIN_ACCOUNT
	{
		u32 connectid;
		u32 clientid;
		char key[USER_MAX_KEY];
		char name[USER_MAX_MEMBER];
		char password[USER_MAX_PASS];
		inline void reset() { memset(this, 0, sizeof(S_LOGIN_ACCOUNT)); };
	};

	void sendErrInfo(const s32 connectid, const u32 cmd, const u16 childcmd);
#pragma pack(pop, packing)

}
#endif