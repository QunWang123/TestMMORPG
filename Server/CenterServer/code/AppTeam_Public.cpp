#include "AppTeam.h"

#include  "CommandData.h"
#include  "ShareFunction.h"
#include  "UserManager.h"
#include  "WorldBC.h"

namespace app
{

	//1 ����λ�ò����Ƿ�ӳ�
	bool S_TEAM_BASE::isLeader(s8 mateindex)
	{
		if (mateindex < 0 || mateindex >= C_TEAM_PLAYERMAX) return false;
		S_TEAM_MATE* mate = &mates[mateindex];
		if (!mate->isused) return false;
		return mate->isleader;
	}
	//2 ���ҳ�Ա
	S_TEAM_MATE* S_TEAM_BASE::findMate(s8 mateindex)
	{
		if (mateindex < 0 || mateindex >= C_TEAM_PLAYERMAX) return nullptr;
		if (!mates[mateindex].isused) return nullptr;
		return  &mates[mateindex];
	}
	//3 ���Ҷӳ�
	S_TEAM_MATE* S_TEAM_BASE::findLeader()
	{
		for (u8 k = 0; k < C_TEAM_PLAYERMAX; k++)
		{
			S_TEAM_MATE* mate = &mates[k];
			if (!mate->isused) continue;
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
			if (mate->isused == false) continue;
			num++;
		}
		return num;
	}
	//�����¶�������
	void S_TEAM_BASE::setNewUserTeam(u32 userindex, s8 mateindex, bool isleader)
	{
		S_USER_TEAM* user = __UserManager->findUser(userindex);
		if (user == nullptr) return;
		if (mateindex < 0 || mateindex >= C_TEAM_PLAYERMAX) return;

		isused = true;
		S_TEAM_MATE* mate = &mates[mateindex];
		mate->userindex = userindex;
		mate->mateindex = mateindex;
		mate->isleader = isleader;
		mate->memid = user->mem.id;
		mate->isused = true;
	}
	//�����¶ӳ�
	S_TEAM_MATE* S_TEAM_BASE::setNewLeader()
	{
		for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
		{
			S_TEAM_MATE* mate = &mates[i];
			if (!mate->isused) continue;
			mate->isleader = true;
			return mate;
		}
		return nullptr;
	}
	//�����¶ӳ�
	S_TEAM_MATE* S_TEAM_BASE::setNewLeader(s8 mateindex)
	{
		if (mateindex < 0 || mateindex >= C_TEAM_PLAYERMAX) return nullptr;

		S_TEAM_MATE* mate = &mates[mateindex];
		if (!mate->isused) return nullptr;
		mate->isleader = true;
		return mate;
	}
	//����һ�����г�Աλ��
	S_TEAM_MATE* S_TEAM_BASE::findEmptyMate()
	{
		for (u8 k = 0; k < C_TEAM_PLAYERMAX; k++)
		{
			S_TEAM_MATE* mate = &mates[k];
			if (mate->isused) continue;
			mate->mateindex = k;
			return mate;
		}
		return nullptr;
	}

	//*********************************************************************
	//*********************************************************************
	//����һ֧���еĶ���
	S_TEAM_BASE* AppTeam::findEmpty()
	{
		for (u32 i = 1; i < C_TEAM_MAX; i++)
		{
			S_TEAM_BASE* team = findTeam(i);
			if (team->isused) continue;

			team->index = i;
			return team;
		}

		return nullptr;
	}



	//��ȡ��������
	u32 AppTeam::getTeamNum()
	{
		u32 num = 0;
		for (u32 i = 0; i < C_TEAM_MAX; i++)
		{
			S_TEAM_BASE* team = findTeam(i);
			if (team->isused) num++;
		}
		return num;
	}

	////�����˺�id 
	S_TEAM_BASE* AppTeam::findTeamMate(const s64& memid, s8& mateindex)
	{
		for (u32 i = 0; i < C_TEAM_MAX; i++)
		{
			S_TEAM_BASE* team = findTeam(i);
			if (team == nullptr) continue;
			if (!team->isused) continue;

			for (u8 k = 0; k < C_TEAM_PLAYERMAX; k++)
			{
				S_TEAM_MATE* mate = &team->mates[k];
				if (mate->memid != memid) continue;

				mateindex = k;
				return team;
			}
		}
		return nullptr;
	}


	//����뿪���� 
	//kind 0-�����뿪 1-�����뿪
	void AppTeam::setTeamLeave(S_USER_TEAM* user, u32 kind)
	{
		if (user == nullptr) return;
		if (user->tmp.temp_TeamIndex < 0) return;

		S_TEAM_BASE* team = __AppTeam->findTeam(user->tmp.temp_TeamIndex);
		if (team == nullptr)
		{
			LOG_MESSAGE("AppTeam setTeamLeave  error...line:%d \n", __LINE__);

			// io::pushLog(io::EFT_RUN, "setTeamLeave err...not findTeam:%d\n", user->tmp.temp_TeamIndex);
			return;
		}
		S_TEAM_MATE* mate = team->findMate(user->tmp.temp_MateIndex);
		if (mate == nullptr || mate->userindex != user->tmp.userindex || mate->memid != user->mem.id)
		{
			LOG_MESSAGE("AppTeam setTeamLeave  error...line:%d \n", __LINE__);
			// io::pushLog(io::EFT_RUN, "setTeamLeave err...!= %d\n", user->tmp.temp_TeamIndex);
			return;
		}
		// 		io::pushLog(io::EFT_RUN, "setTeamLeave name:%s id:%d/%lld  line:%d/%d team:%d/%d kind:%d\n",
		// 			user->mem.name, user->tmp.userindex, user->mem.id,
		// 			user->tmp.line, user->status.mapid, user->tmp.temp_TeamIndex, user->tmp.temp_MateIndex, kind);

		LOG_MESSAGE("setTeamLeave....%lld kind:%d \n", user->mem.id, kind);

		bool isleader = mate->isleader;
		//�㲥����뿪����
		bc_GateServer_TeamLeave(team->index, user->tmp.temp_MateIndex);
		mate->reset();

		//����ֻ��1��
		if (team->getNum() == 0)
		{
			LOG_MESSAGE("setTeamLeave  team init...%d memid:%lld team:%d\n", user->tmp.userindex, user->mem.id, team->index);

			//���Ǹ������뿪
			if (kind != EDT_COPY)  nf_GameServer_TeamLeave(user);
			user->tmp.setTeamIndex(-1, -1);
			bc_GameServer_TeamData(team->index);
			team->reset();
			return;
		}

		if (isleader)
		{
			//�����µĶӳ�
			S_TEAM_MATE* newleader = team->setNewLeader();
			if (newleader == nullptr)
			{
				LOG_MESSAGE("AppTeam set newleader is error...line:%d \n", __LINE__);
				return;
			}
			S_USER_TEAM* leader_user = __UserManager->findUser(newleader->userindex, newleader->memid);
			if (leader_user == nullptr)
			{
				LOG_MESSAGE("AppTeam set newleader is error...line:%d \n", __LINE__);
				return;
			}
			//�㲥����ҳ�Ϊ�ӳ�
			bc_GateServer_TeamLeader(team->index, newleader->mateindex);
		}

		//֪ͨ��Ϸ������ ĳ���뿪���� �������ݱ仯
		bc_GameServer_TeamData(team->index);
		if (kind != EDT_COPY) nf_GameServer_TeamLeave(user);

		user->tmp.setTeamIndex(-1, -1);
	}
}