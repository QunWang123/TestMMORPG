#include  "WorldBC.h"
#include  "AppTeam.h"
#include  "CommandData.h"
#include  "ShareFunction.h"
#include  "UserManager.h"



namespace app
{

	
	//************************************************************************
	//team
	//gateserver**************************************************************

	void nf_GateServer_TeamSelf(S_USER_TEAM* user, const u32 cid,bool isleader)
	{
		//1��֪ͨ�Լ�������� 
		S_TEAM_CREATE data;
		data.user_connectid = user->tmp.user_connectid;
		data.memid = user->mem.id;
		data.userindex = user->tmp.userindex;
		data.isleader = isleader;
		data.teamindex = user->tmp.temp_TeamIndex;
		data.mateindex = user->tmp.temp_MateIndex;

		__TcpServer->begin(cid, CMD_7120);
		__TcpServer->sss(cid, &data, sizeof(S_TEAM_CREATE));
		__TcpServer->end(cid);


	}
	//�㲥ĳ����Ա�뿪
	void bc_GateServer_TeamLeave(const u32 teamindex, u8 mateindex)
	{
		S_TEAM_BASE* team = __AppTeam->findTeam(teamindex);
		if (team == nullptr) return;
		if (!team->isused) return;
		//S_TEAM_MATE* leave_mate = team->findMate(mateindex);
		//if (leave_mate == nullptr) return;

		for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
		{
			S_TEAM_MATE* mate = team->findMate(i);
			if (mate == nullptr) continue;
			S_USER_TEAM* user = __UserManager->findUser(mate->userindex, mate->memid);
			if (user == nullptr) continue;

			net::S_CLIENT_BASE * c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
			if (c == nullptr) continue;

			S_TEAM_LEAVE data;
			data.user_connectid = user->tmp.user_connectid;
			data.memid = user->mem.id;
			data.userindex = user->tmp.userindex;
			data.mateindex = mateindex;

			LOG_MESSAGE("7040 ��������뿪����....index:%d/%lld/%d %d \n", user->tmp.userindex, user->mem.id, user->tmp.user_connectid,mateindex);

			//֪ͨĳ����Ա�뿪
			__TcpServer->begin(c->ID, CMD_7040);
			__TcpServer->sss(c->ID, (u16)0);
			__TcpServer->sss(c->ID, &data,sizeof(S_TEAM_LEAVE));
			__TcpServer->end(c->ID);
		}
	}
	//�㲥ĳ����Ա��Ϊ�ӳ�
	void bc_GateServer_TeamLeader(const u32 teamindex,u8 mateindex)
	{
		S_TEAM_BASE* team = __AppTeam->findTeam(teamindex);
		if (team == nullptr) return;
		if (!team->isused) return;

		for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
		{
			S_TEAM_MATE* mate = team->findMate(i);
			if (mate == nullptr) continue;
			S_USER_TEAM* user = __UserManager->findUser(mate->userindex, mate->memid);
			if (user == nullptr) continue;

			net::S_CLIENT_BASE* c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
			if (c == nullptr) continue;

			//ʹ������ṹ�� ����λ�ó�Ϊ�˶ӳ�
			S_TEAM_LEAVE data;
			data.user_connectid = user->tmp.user_connectid;
			data.memid = user->mem.id;
			data.userindex = user->tmp.userindex;
			data.mateindex = mateindex;


			__TcpServer->begin(c->ID, CMD_7110);
			__TcpServer->sss(c->ID, &data, sizeof(S_TEAM_LEAVE));
			__TcpServer->end(c->ID);
		}
	}
	//�㲥 ĳ�˼����˶���
	void bc_GateServer_TeamJoin(net::S_CLIENT_BASE* c, S_USER_TEAM* user, S_TEAM_BASE* team)
	{
		if (user == nullptr) return;
		if (c == nullptr)  return;
		if (team == nullptr || !team->isused) return;
		S_TEAM_MATE* mate = team->findMate(user->tmp.temp_MateIndex);
		if (mate == nullptr) return;

		//1��������3�˵���Ϣ�����Լ�
		for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
		{
			S_TEAM_MATE* mate_other = team->findMate(i);
			if (mate_other == nullptr) continue;
			if (mate_other->memid == user->mem.id) continue;

			S_USER_TEAM* mateuser = __UserManager->findUser(mate_other->userindex, mate_other->memid);
			if (mateuser == nullptr) continue;

			//�Ѷ��ѵ����ݷ����Լ�
			S_TEAM_INFO info;
			info.user_connectid = user->tmp.user_connectid;
			info.memid = user->mem.id;
			info.userindex = user->tmp.userindex;
			info.teamindex = team->index;
			info.materindex = i;
			info.job = mateuser->role.job;
			info.level = mateuser->role.level;
			info.sex = mateuser->role.sex;
			info.job = mateuser->role.job;
			info.other_index = mateuser->tmp.userindex;
			info.other_memid  = mateuser->mem.id;
			info.isleader = mate_other->isleader;
			memcpy(info.nick, mateuser->role.nick, USER_MAX_NICK);

			__TcpServer->begin(c->ID, CMD_7100);
			__TcpServer->sss(c->ID, &info,sizeof(S_TEAM_INFO));
			__TcpServer->end(c->ID);

			//���Լ������ݷ�������
			S_TEAM_INFO info2;
			info2.user_connectid = mateuser->tmp.user_connectid;
			info2.memid = mateuser->mem.id;
			info2.userindex = mateuser->tmp.userindex;
			info2.teamindex = team->index;
			info2.materindex = user->tmp.temp_MateIndex;
			info2.job = user->role.job;
			info2.level = user->role.level;
			info2.sex = user->role.sex;
			info2.job = user->role.job;
			info2.other_index = user->tmp.userindex;
			info2.other_memid = user->mem.id;
			info2.isleader = mate->isleader;
			memcpy(info2.nick, user->role.nick, USER_MAX_NICK);

			net::S_CLIENT_BASE* other_c = __TcpServer->client(mateuser->tmp.server_connectid, mateuser->tmp.server_clientid);
			if (other_c == nullptr)  continue;
			__TcpServer->begin(other_c->ID, CMD_7100);
			__TcpServer->sss(other_c->ID, &info2, sizeof(S_TEAM_INFO));
			__TcpServer->end(other_c->ID);
		}

	}
	void bc_GameServer_TeamData()
	{
		if (__AppTeam == nullptr) return;

		for (u32 i = 0; i < C_TEAM_MAX; i++)
		{
			S_TEAM_BASE* team = __AppTeam->findTeam(i);
			if (team->isused == false) continue;

			bc_GameServer_TeamData(i);
		}
	}
	//************************************************************************
	//gameserver**************************************************************
	//�㲥�������ݱ仯 7500
	void bc_GameServer_TeamData(const u32 teamindex)
	{
		S_TEAM_BASE* team = __AppTeam->findTeam(teamindex);
		if (team == nullptr) return;
		//���ݱ仯 �㲥������Ϸ������
		u32  num = func::__ServerInfo->MaxConnect;
		for (u32 i = 0; i < num; i++)
		{
			net::S_CLIENT_BASE* c = __TcpServer->client(i);
			if (c == nullptr) continue;
			if (c->clientType != func::S_TYPE_GAME) continue;
			if (c->clientID == 0)  continue;
			if (c->state < func::S_CONNECT_SECURE) continue;

			__TcpServer->begin(c->ID, CMD_7500);
			__TcpServer->sss(c->ID, teamindex);
			__TcpServer->sss(c->ID, team, sizeof(S_TEAM_BASE));//����ṹ��
			__TcpServer->end(c->ID);
		}
	}
	//֪ͨ����뿪����
	void nf_GameServer_TeamLeave(S_USER_TEAM* user)
	{
		net::S_CLIENT_BASE* c = findGameServer_Connection(user->tmp.line, user->status.mapid);
		if (c == nullptr) 
		{
			LOG_MESSAGE("nf_GameServer_TeamLeave c=nullptr...%d %d \n", user->tmp.line, user->status.mapid);
			return;
		}
		if (c->clientType != func::S_TYPE_GAME) 
		{
			LOG_MESSAGE("nf_GameServer_TeamLeave err...%d %d %d \n", user->tmp.line, user->status.mapid,c->clientType);
			return;
		}

		__TcpServer->begin(c->ID, CMD_7040);
		__TcpServer->sss(c->ID, user->tmp.userindex);
		__TcpServer->sss(c->ID, user->mem.id);
		__TcpServer->end(c->ID);
	}

	//7600 ֪ͨ��� �����Լ��Ķ������� ok
	void nf_GameServer_TeamSelf(S_USER_TEAM* user)
	{
		if (user == nullptr) return;

		//���ݵ�ͼID���ҵ����������Ϸ������
		net::S_CLIENT_BASE* c = findGameServer_Connection(user->tmp.line,user->status.mapid);
		if (c == nullptr)
		{
			LOG_MESSAGE("nf_GameServer_TeamSelf c=nullptr...%d %d \n", user->tmp.line, user->status.mapid);
			return;
		}
		if (c->clientType != func::S_TYPE_GAME)
		{
			LOG_MESSAGE("nf_GameServer_TeamSelf err...%d %d %d", user->tmp.line, user->status.mapid, c->clientType);
			return;
		}

		__TcpServer->begin(c->ID, CMD_7600);
		__TcpServer->sss(c->ID, user->tmp.userindex);
		__TcpServer->sss(c->ID, user->mem.id);
		__TcpServer->sss(c->ID, user->tmp.temp_TeamIndex);
		__TcpServer->sss(c->ID, user->tmp.temp_MateIndex);
		__TcpServer->end(c->ID);
	}

	//***********************************************************************************
	//����copy
	//�㲥 ��Ϸ������  ������ҽ��븱����ͼ ��ʼ�л���ͼ

	//void bc_GameServer_CopyChangeMap(u32 teamid,u32 mapid)
	//{
	//	S_TEAM_ROOM * team = __AppTeam->Room(teamid);
	//	if (team == nullptr || !team->isused) return;

	//	for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
	//	{
	//		S_TEAM_ROOM_BASE* mate = team->FindMate(i);
	//		if (mate == nullptr || !mate->isused) continue;

	//		nf_GameServer_CopyChangeMapOne(mate->centerindex,  mate->memid, mapid);
	//	}
	//}
	////9000-1000 ֪ͨ��Ϸ������ ��ʼ�л���ͼ
	//void nf_GameServer_CopyChangeMapOne(u32 centerindex, n64 memid,u32 mapid)
	//{
	//	S_USER_TEAM* user = __UserManager->FindUser(centerindex, memid);
	//	if (user == nullptr) 
	//	{
	//		debug(func::DB_BOTH, "WorldBC iserror...line:%d \n",  __LINE__);
	//		return;
	//	}
	//	net::S_CLIENT_BASE* c = FindGameServer(user->Role.mapid, __TcpServer);
	//	if (c == nullptr) 
	//	{
	//		debug(func::DB_BOTH, "WorldBC iserror...line:%d \n", __LINE__);
	//		return;
	//	}
	//	debug(func::DB_BOTH, "��ʼ֪ͨ��� ׼���л���ͼ..%d line:%d \n", user->TMP.GameIndex, __LINE__);

	//	__TcpServer->b(c->ID, CMD_COPY_9000);
	//	__TcpServer->s(c->ID, (u16)6000);
	//	__TcpServer->s(c->ID, user->TMP.GameIndex);
	//	__TcpServer->s(c->ID, user->MEM.ID);
	//	__TcpServer->s(c->ID, mapid); 
	//	__TcpServer->e(c->ID);
	//}
	////9000-1000 ֪ͨ��Ϸ������ ������ʼ

	//void nf_GameServer_CopyStart(void* _room)
	//{
	//	S_COPY_ROOM* room = (S_COPY_ROOM*)_room;
	//	if (room == nullptr) return;
	//	net::S_CLIENT_BASE* c = FindGameServer(room->mapid, __TcpServer);
	//	if (c == nullptr) return;
	//	
	//	__TcpServer->b(c->ID, CMD_COPY_9000);
	//	__TcpServer->s(c->ID, (u16)1000);
	//	__TcpServer->s(c->ID, room,sizeof(S_COPY_ROOM));
	//	__TcpServer->e(c->ID);
	//}
	////9000-3000 ֪ͨ��Ϸ������ ��ҵ��߸�������
	//void nf_GameServer_CopyOver(u32 copyid,u32 mapid)
	//{
	//	net::S_CLIENT_BASE* c = FindGameServer(mapid, __TcpServer);
	//	if (c == nullptr) return;
	//	__TcpServer->b(c->ID, CMD_COPY_9000);
	//	__TcpServer->s(c->ID, (u16)3000);
	//	__TcpServer->s(c->ID, copyid);
	//	__TcpServer->e(c->ID);
	//}
	////9000-5000 ֪ͨ��Ϸ������ ��ҽ��븱����
	//void nf_GameServer_CopyEntry(u32 mapid,u32 gameindex, n64 mid, n32 copyid)
	//{
	//	net::S_CLIENT_BASE* c = FindGameServer(mapid, __TcpServer);
	//	if (c == nullptr) return;
	//	__TcpServer->b(c->ID, CMD_COPY_9000);
	//	__TcpServer->s(c->ID, (u16)5000);
	//	__TcpServer->s(c->ID, gameindex);
	//	__TcpServer->s(c->ID, mid);
	//	__TcpServer->s(c->ID, copyid);
	//	__TcpServer->e(c->ID);
	//}


}
