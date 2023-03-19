#ifndef  ____APPTEAM_H
#define  ____APPTEAM_H

#include  "APPGlobal.h"
#include "UserData.h"

namespace app
{
	class  AppTeam :public IContainer
	{
	private:
		HashArray<S_TEAM_BASE>* __Teams;
	public:
		AppTeam();
		virtual ~AppTeam();
		virtual void __cdecl onInit();
		virtual bool __cdecl onClientCommand(net::ITCPClient* tc, const u16 cmd);
	public:
		inline S_TEAM_BASE* findTeam(const s32 id)
		{
			if (id >= __Teams->length || id <= 0) return nullptr;
			return __Teams->Value(id);
		}
		inline 	u32 getTeamNum()
		{
			u32 num = 0;
			for (u32 i = 0; i < C_TEAM_MAX; i++)
			{
				S_TEAM_BASE*  team = findTeam(i);
				if (team->isused) num++;
			}
			return num;
		}
		void reset();
		S_TEAM_BASE* findTeam(const s64 memid,s32& teamindex,s8& mateindex);
	
	};

	extern AppTeam*  __AppTeam;
}
#endif