#ifndef  ____APPTEAM_H
#define  ____APPTEAM_H


#include "AppGlobal.h"
#include "WorldData.h"

namespace app
{
	class  AppTeam :public IContainer
	{
	private:
		HashArray<S_TEAM_BASE>*	__Teams;
	public:
		AppTeam();
		virtual ~AppTeam();
		virtual void __cdecl onInit();
		virtual void __cdecl onUpdate();
		virtual bool __cdecl onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);

	public:
		inline S_TEAM_BASE*  findTeam(const s32& id)
		{
			if (id >= __Teams->length || id < 0) return nullptr;
			return __Teams->Value(id);
		}
		inline S_TEAM_MATE* findMate(const s32& id,const s8& mateindex)
		{
			auto team = findTeam(id);
			if (team == nullptr) return nullptr;
			auto mate = team->findMate(mateindex);
			return mate;
		}
		S_TEAM_BASE* findEmpty();
		S_TEAM_BASE* findTeamMate(const s64 &memid,s8 &mateindex);
		u32 getTeamNum();
		void setTeamLeave(S_USER_TEAM* user,u32 kind);
	};

	extern AppTeam*  __AppTeam;
	extern void onStartCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c);
	extern void onLeaveCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c);
}
#endif