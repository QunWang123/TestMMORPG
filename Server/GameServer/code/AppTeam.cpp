
#include "AppTeam.h"

#include  "CommandData.h"
#include "WorldMap.h"
#include "WorldBC.h"

#include "WorldTools.h"
#include "WorldScript.h"
#include  "UserManager.h"

namespace app
{

	AppTeam*  __AppTeam = nullptr;
	AppTeam::AppTeam()
	{
		onInit();
	}

	AppTeam::~AppTeam()
	{
	}

	void AppTeam::onInit()
	{
		__Teams = new HashArray<S_TEAM_BASE>(C_TEAM_MAX);
		reset();
	}
	void  AppTeam::reset()
	{
		for (u32 i = 0; i < C_TEAM_MAX; i++)
		{
			S_TEAM_BASE* team = findTeam(i);
			if (team == nullptr) continue;
			team->reset();
		}
	}

	S_TEAM_BASE* AppTeam::findTeam(const s64 memid,s32& teamindex,s8& mateindex)
	{
		u32 num = 0;
		for (u32 i = 1; i < C_TEAM_MAX; i++)
		{
			S_TEAM_BASE* team = findTeam(i);
			if (!team->isused) continue;
			
			for (u32 j = 0; j < C_TEAM_PLAYERMAX; j++)
			{
				auto mate = team->findMate(j);
				if (!mate->isused) continue;
				if (mate->memid == memid)
				{
					teamindex = i;
					mateindex = j;
					return team;
				}
			}
		}
		return nullptr;
	}

	//**********************************************************
	//**********************************************************
	//7040 离开了某个队伍
	void  c_TeamLeave(net::ITCPClient* tc)
	{
		u32  userindex = 0;
		u64  memid = 0;
		tc->read(userindex);
		tc->read(memid);

		S_USER_BASE*  user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("AppTeam c_TeamSelf is error...%d %lld line:%d \n", userindex, memid,__LINE__);
			return;
		}

		//user->leaveCopy(false);
		user->tmp.setTeamIndex(-1,-1);
		user->node.layer = -1;//设置为公共地图层级 -1

		LOG_MESSAGE("[TeasmServer]  xx离开队伍 ...%d memid:%lld \n", userindex, user->mem.id);
	}

	//7500 收到team服务器 队伍数据更新
	void  c_TeamData(net::ITCPClient* tc)
	{
		u32 teamindex = 0;
		S_TEAM_BASE data;
		tc->read(teamindex);
		tc->read(&data, sizeof(S_TEAM_BASE));

		S_TEAM_BASE* team = __AppTeam->findTeam(teamindex);
		if (team == nullptr)
		{
			LOG_MESSAGE("AppTeam  c_TeamData is error...line:%d \n", __LINE__);
			return;
		}

		memcpy(team, &data, sizeof(S_TEAM_BASE));

		LOG_MESSAGE("[TeasmServer]  队伍数据刷新 ...team:%d/%d \n", teamindex, team->getNum());

	}
	//CMD_7600 自己加入了某个队伍
	void  c_TeamSelf(net::ITCPClient* tc)
	{
		s32 teamindex = 0;
		u8  mateindex = 0;
		u32  userindex = 0;
		u64  memid = 0;
		tc->read(userindex);
		tc->read(memid);
		tc->read(teamindex);
		tc->read(mateindex);

		S_USER_BASE*  user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("AppTeam c_TeamSelf is error...line:%d \n", __LINE__);
			return;
		}


		user->tmp.setTeamIndex(teamindex, mateindex);

		LOG_MESSAGE("[TeasmServer]  xx自己队伍数据 ...%d memid:%lld team:%d/%d \n",
			userindex, user->mem.id, user->tmp.temp_TeamIndex, user->tmp.temp_MateIndex);
	}
	
	bool AppTeam::onClientCommand(net::ITCPClient * tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_CENTER) return false;

		switch (cmd)
		{
		case CMD_7040:c_TeamLeave(tc); break;
		case CMD_7500:c_TeamData(tc);  break;
		case CMD_7600:c_TeamSelf(tc);  break;
		}

		return true;
	}

	//*****************************************************************
	//*****************************************************************
	//*****************************************************************
	//1 根据位置查找是否队长
	bool S_TEAM_BASE::isLeader(s8 mateindex)
	{
		if (mateindex < 0 || mateindex >= C_TEAM_PLAYERMAX) return false;
		S_TEAM_MATE* mate = &mates[mateindex];
		if (!mate->isused) return false;
		return mate->isleader;
	}
	//2 查找成员
	S_TEAM_MATE* S_TEAM_BASE::findMate(s8 mateindex)
	{
		if (mateindex < 0 || mateindex >= C_TEAM_PLAYERMAX) return nullptr;
		if (!mates[mateindex].isused) return nullptr;
		return  &mates[mateindex];
	}
	//3 查找队长
	S_TEAM_MATE* S_TEAM_BASE::findLeader()
	{
		for (u8 k = 0; k < C_TEAM_PLAYERMAX; k++)
		{
			S_TEAM_MATE* mate = &mates[k];
			if (mate->isused) continue;
			if (!mate->isleader) continue;
			return mate;
		}
		return nullptr;
	}
	u32 S_TEAM_BASE::getNum()
	{
		u32 num = 0;
		for (u8 k = 0; k < C_TEAM_PLAYERMAX; k++)
		{
			S_TEAM_MATE* mate = &mates[k];
			if (!mate->isused) continue;
			num++;
		}
		return num;
	}
}


