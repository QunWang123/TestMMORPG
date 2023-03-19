#include "AppTeam.h"

#include  "CommandData.h"
#include  "ShareFunction.h"
#include  "UserManager.h"
#include  "WorldBC.h"
// #include <LogFile.h>
#include  "WorldScript.h"

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

		for (u32 i = 0; i < C_TEAM_MAX; i++)
		{
			S_TEAM_BASE*  team = findTeam(i);
			team->reset();
		}
	}

	void AppTeam::onUpdate()
	{

	}
	




	//7000 ��������
	void onTeamCreate(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_TEAM_CREATE data;
		ts->read(c->ID, &data, sizeof(S_TEAM_CREATE));

		//1��û���ҵ�����
		//�����ķ���������û���������
		S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7000, 7001, &data, sizeof(S_TEAM_CREATE));
			return;
		}
		//2���ж����� �±�λ�ô�1��ʼ 0����-1Ĭ��Ϊû�ж���
		if (user->tmp.temp_TeamIndex >= C_TEAM_START_POS)
		{
			sendErrInfo(c->ID, CMD_7000, 7002, &data, sizeof(S_TEAM_CREATE));
			return;
		}
		//3����֤�Ƿ��ڸ����ڣ������ڲ���������κβ��� ��;�뿪����=�˳�����
		bool iscopy = script::isCopyMap(user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7000, 7003, &data, sizeof(S_TEAM_CREATE));
			return;
		}

		if (__AppTeam == nullptr) return;

		//3��Ѱ��һ֧���еĶ��飿
		S_TEAM_BASE * team = __AppTeam->findEmpty();
		if (team == nullptr)
		{
			//���鷿������ 
			sendErrInfo(c->ID, CMD_7000, 2004, &data, sizeof(S_TEAM_CREATE));
			return;
		}

		//���ö��鴴��ʱ��
		team->isused = true;
		team->createtime = time(NULL);
		//���ö������� ���������ͳ�Ա����
		user->tmp.temp_TeamIndex = team->index;
		user->tmp.temp_MateIndex = 0;

		team->setNewUserTeam(data.userindex, 0, true);
		//���ö���λ��
		data.teamindex = team->index; //����ID
		data.mateindex = 0; //��ԱID
		data.isleader = true;//�Ƿ�ӳ�

		//1������gateserver ֪ͨ�Լ��������
		nf_GateServer_TeamSelf(user, c->ID, true);
		//2���㲥����gameserver
		//gameserverҲ��Ҫͬ���������� ��gameserver�ͷ���ͬ�����������Ϸ����
		//���еĲ�����teamserver������ѯ������gameserver      
		//7100 ֪ͨgameserver �����Լ��Ķ�����Ϣ
		nf_GameServer_TeamSelf(user);
		//7200 ͬ������Ϸ������ ��������
		bc_GameServer_TeamData(team->index);

		LOG_MESSAGE("onTeamCreate success...%d memid:%lld team:%d connectid:%d\n",
			user->tmp.userindex, user->mem.id, team->index, data.user_connectid);

	}
	//7010 ����xxx���
	void onTeamPlease(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_TEAM_PLEASE data;
		ts->read(c->ID, &data, sizeof(S_TEAM_PLEASE));

		//1��û���ҵ�����
		S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7010, 7001, &data, sizeof(S_TEAM_PLEASE));
			return;
		}
		//2����ѯ�Լ���û�ж��飿
		if (user->tmp.temp_TeamIndex < C_TEAM_START_POS)
		{
			sendErrInfo(c->ID, CMD_7010, 7002, &data, sizeof(S_TEAM_PLEASE));
			return;
		}

		//4���Լ������Լ�
		if (data.memid == data.other_memid)
		{
			sendErrInfo(c->ID, CMD_7010, 7003, &data, sizeof(S_TEAM_PLEASE));
			return;
		}

		//3�����ұ�����������
		S_USER_TEAM* other_user = __UserManager->findUser(data.other_userindex, data.other_memid);
		if (other_user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7010, 7004, &data, sizeof(S_TEAM_PLEASE));
			return;
		}
		//3�����������ж�����
		if (other_user->tmp.temp_TeamIndex >= C_TEAM_START_POS)
		{
			sendErrInfo(c->ID, CMD_7010, 7005, &data, sizeof(S_TEAM_PLEASE));
			return;
		}
		//3����֤�Ƿ��ڸ����ڣ������ڲ���������κβ��� ��;�뿪����=�˳�����
		bool iscopy = script::isCopyMap(user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7010, 7006, &data, sizeof(S_TEAM_PLEASE));
			return;
		}
		iscopy = script::isCopyMap(other_user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7010, 7007, &data, sizeof(S_TEAM_PLEASE));
			return;
		}
		//4 ����xxx�������ط����� ������? 
		net::S_CLIENT_BASE* other_c = __TcpServer->client(other_user->tmp.server_connectid, other_user->tmp.server_clientid);
		if (other_c == nullptr)
		{
			sendErrInfo(c->ID, CMD_7010, 7008, &data, sizeof(S_TEAM_PLEASE));
			return;
		}

		//6 ��ѯ�ж��黹���޶��� 
		S_TEAM_BASE * team = __AppTeam->findTeam(user->tmp.temp_TeamIndex);
		if (team == nullptr || !team->isused)
		{
			sendErrInfo(c->ID, CMD_7010, 7009, &data, sizeof(S_TEAM_PLEASE));
			return;
		}
		//8 ���Ƕӳ� ��������
		if (!team->isLeader(user->tmp.temp_MateIndex))
		{
			sendErrInfo(c->ID, CMD_7010, 7010, &data, sizeof(S_TEAM_PLEASE));
			return;
		}

		//10 ��ѯ�����Ա����������
		S_TEAM_MATE * mate = team->findEmptyMate();
		if (mate == nullptr)
		{
			sendErrInfo(c->ID, CMD_7010, 7011, &data, sizeof(S_TEAM_PLEASE));
			return;
		}

		//�������ݸ���������xxx
		S_TEAM_PLEASE data2;
		data2.user_connectid = other_user->tmp.user_connectid;
		data2.memid = other_user->mem.id;
		data2.userindex = other_user->tmp.userindex;
		data2.other_userindex = user->tmp.userindex;
		data2.other_memid = user->mem.id;

		memset(data2.nick, 0, USER_MAX_NICK);
		memcpy(data2.nick, user->role.nick, USER_MAX_NICK);

		//xxxx ���㷢���������
		__TcpServer->begin(other_c->ID, CMD_7010);
		__TcpServer->sss(other_c->ID, (u16)0);
		__TcpServer->sss(other_c->ID, &data2, sizeof(S_TEAM_PLEASE));
		__TcpServer->end(other_c->ID);
	}
	//7020 ͬ�����xxx�Ķ���
	void onTeamJoin(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_TEAM_AGREE data;
		ts->read(c->ID, &data, sizeof(S_TEAM_AGREE));

		//1��û���ҵ�����
		S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7020, 7001, &data, sizeof(S_TEAM_AGREE));
			LOG_MESSAGE("onTeamJoin err...user=null %d/%lld\n", data.userindex, data.memid);
			return;
		}

		//2�����Ҷӳ�����
		S_USER_TEAM* leader_user = __UserManager->findUser(data.leaderindex, data.leadermemid);
		if (leader_user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7020, 7002, &data, sizeof(S_TEAM_AGREE));
			LOG_MESSAGE("onTeamJoin err...leader_user=null %d/%lld\n", data.leaderindex, data.leadermemid);
			return;
		}
		//3����֤�Ƿ��ڸ����ڣ������ڲ���������κβ��� ��;�뿪����=�˳�����
		bool iscopy = script::isCopyMap(user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7010, 7003, &data, sizeof(S_TEAM_AGREE));
			return;
		}
		iscopy = script::isCopyMap(leader_user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7010, 7004, &data, sizeof(S_TEAM_AGREE));
			return;
		}
		//3����֤�ӳ����Լ�
		if (data.memid == data.leadermemid)
		{
			sendErrInfo(c->ID, CMD_7020, 7005, &data, sizeof(S_TEAM_AGREE));
			LOG_MESSAGE("onTeamJoin err...leadermemid=mymemid %d/%lld\n", data.leaderindex, data.leadermemid);
			return;
		}
		//3����֤����
		s8 mateindex = 0;
		S_TEAM_BASE * team = __AppTeam->findTeam(leader_user->tmp.temp_TeamIndex);
		if (team == nullptr || !team->isused)
		{
			sendErrInfo(c->ID, CMD_7020, 7005, &data, sizeof(S_TEAM_AGREE));
			LOG_MESSAGE("onTeamJoin err...team=null %d\n", leader_user->tmp.temp_TeamIndex);
			return;
		}
		//��֤����������
		auto mate = team->findEmptyMate();
		if (mate == nullptr)
		{
			sendErrInfo(c->ID, CMD_7020, 7006, &data, sizeof(S_TEAM_AGREE));

			LOG_MESSAGE("onTeamJoin err...��Ա���� %d\n", leader_user->tmp.temp_TeamIndex);
			return;
		}

		//���ö�������
		user->tmp.setTeamIndex(team->index, mate->mateindex);
		team->setNewUserTeam(data.userindex, mate->mateindex, false);

		LOG_MESSAGE("onTeamJoin [%s]�����˶��� %d %d\n", user->role.nick, team->index, mate->mateindex);

		//1��7120֪ͨ�Լ�������� 
		//7100 �㲥gaterserver xx�����˶���
		nf_GateServer_TeamSelf(user, c->ID, false);
		bc_GateServer_TeamJoin(c, user, team);
		//2��֪ͨ��Ϸ������
		nf_GameServer_TeamSelf(user);
		bc_GameServer_TeamData(team->index);
	}
	//7030 �ܾ�����ĳ���˵Ķ���
	void onTeamRefuse(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		//S_TEAM_AGREE data;
		//ts->read(c->ID, &data, sizeof(S_TEAM_AGREE));

		////1��û���ҵ�����
		//S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		//if (user == nullptr)
		//{
		//	sendErrInfo(c->ID, CMD_7030, 2001, &data, sizeof(S_TEAM_AGREE));
		//	return;
		//}

		////2�����Ҷӳ�����
		//S_USER_TEAM* leader_user = __UserManager->findUser(data.leaderindex);
		//if (leader_user == nullptr)
		//{
		//	sendErrInfo(c->ID, CMD_7030, 2002, &data, sizeof(S_TEAM_AGREE));
		//	return;
		//}
		////3 ���Ҷӳ�����
		//net::S_CLIENT_BASE* leader_c = __TcpServer->client(leader_user->tmp.server_connectid, leader_user->tmp.server_clientid);
		//if (leader_c == nullptr) return;


		//char nick[USER_MAX_NICK];
		//memset(nick, 0, USER_MAX_NICK);
		//memcpy(nick,  user->role.nick, USER_MAX_NICK);


		////xxxx �ܾ�������������
		//__TcpServer->begin(leader_c->ID, CMD_7030);
		//__TcpServer->sss(leader_c->ID, (u16)0);
		//__TcpServer->sss(leader_c->ID, &data,sizeof(S_TEAM_AGREE));
		//__TcpServer->sss(leader_c->ID, nick, USER_MAX_NICK);
		//__TcpServer->end(leader_c->ID);
	}

	//7040 �뿪����
	void onTeamLeave(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_TEAM_LEAVE data;
		ts->read(c->ID, &data, sizeof(S_TEAM_LEAVE));

		//1��û���ҵ�����
		S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7040, 7001, &data, sizeof(S_TEAM_LEAVE));
			return;
		}

		__AppTeam->setTeamLeave(user, EDT_SELF);
	}


	//7050 �ӳ�T��ĳ��
	void onTeamOut(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_TEAM_KILL data;
		ts->read(c->ID, &data, sizeof(S_TEAM_KILL));

		//1��û���ҵ�����
		S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7050, 7001, &data, sizeof(S_TEAM_KILL));
			return;
		}
		//2����֤�Ƿ��ڸ����ڣ������ڲ���������κβ��� ��;�뿪����=�˳�����
		bool iscopy = script::isCopyMap(user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7050, 7002, &data, sizeof(S_TEAM_KILL));
			return;
		}

		S_TEAM_BASE* team = __AppTeam->findTeam(user->tmp.temp_TeamIndex);
		if (team == nullptr)
		{
			sendErrInfo(c->ID, CMD_7050, 7003, &data, sizeof(S_TEAM_KILL));
			return;
		}
		//2�������ǲ��Ƕӳ���
		bool isleader = team->isLeader(user->tmp.temp_MateIndex);
		if (!isleader)
		{
			sendErrInfo(c->ID, CMD_7050, 7004, &data, sizeof(S_TEAM_KILL));
			return;
		}
		if (user->tmp.temp_MateIndex == data.mateindex)
		{
			sendErrInfo(c->ID, CMD_7050, 7005, &data, sizeof(S_TEAM_KILL));
			return;
		}
		//3��Ѱ�ұ��߶�Ա��λ��
		S_TEAM_MATE* mate = team->findMate(data.mateindex);
		if (mate == nullptr)
		{
			sendErrInfo(c->ID, CMD_7050, 7006, &data, sizeof(S_TEAM_KILL));
			return;
		}

		//1��֪ͨ���ߵ���Ա
		S_USER_TEAM* outuser = __UserManager->findUser(mate->userindex, mate->memid);
		if (outuser != nullptr)
		{
			iscopy = script::isCopyMap(outuser->status.mapid);
			if (iscopy)
			{
				sendErrInfo(c->ID, CMD_7050, 7007, &data, sizeof(S_TEAM_KILL));
				return;
			}

			outuser->tmp.setTeamIndex(-1, -1);
			nf_GameServer_TeamLeave(outuser);
		}

		bc_GateServer_TeamLeave(team->index, data.mateindex);
		bc_GameServer_TeamData(team->index);
		mate->reset();
	}
	//7060 �������xxx�Ķ���
	void onTeamRequest(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_TEAM_REQUEST data;
		ts->read(c->ID, &data, sizeof(S_TEAM_REQUEST));

		//1��û���ҵ�����
		S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7060, 7001, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		//2 ���Ѿ��ж�����
		if (user->tmp.temp_TeamIndex >= C_TEAM_START_POS)
		{
			sendErrInfo(c->ID, CMD_7060, 7002, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		//3��û���ҵ�����
		S_USER_TEAM* user_other = __UserManager->findUser(data.otherindex, data.othermemid);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7060, 7003, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		//2����֤�Ƿ��ڸ����ڣ������ڲ���������κβ��� ��;�뿪����=�˳�����
		bool iscopy = script::isCopyMap(user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7060, 7004, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		//2����֤�Ƿ��ڸ����ڣ������ڲ���������κβ��� ��;�뿪����=�˳�����
		iscopy = script::isCopyMap(user_other->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7060, 7005, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		//4 �����û�ж���
		if (user_other->tmp.temp_TeamIndex < C_TEAM_START_POS)
		{
			sendErrInfo(c->ID, CMD_7060, 7006, &data, sizeof(S_TEAM_REQUEST));
			return;
		}

		//5 û�ж��� ����û������
		S_TEAM_BASE* team = __AppTeam->findTeam(user_other->tmp.temp_TeamIndex);
		if (team == nullptr || !team->isused)
		{
			sendErrInfo(c->ID, CMD_7060, 7007, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		//6����ѯ��������������
		S_TEAM_MATE* mate = team->findEmptyMate();
		if (mate == nullptr)
		{
			sendErrInfo(c->ID, CMD_7060, 7008, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		//7��Ѱ�Ҷӳ�
		S_TEAM_MATE* leader_mate = team->findLeader();
		if (leader_mate == nullptr)
		{
			sendErrInfo(c->ID, CMD_7060, 7009, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		//8�����Ҷӳ�����
		S_USER_TEAM* leader_user = __UserManager->findUser(leader_mate->userindex, leader_mate->memid);
		if (leader_user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7060, 7010, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		iscopy = script::isCopyMap(leader_user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7060, 7011, &data, sizeof(S_TEAM_REQUEST));
			return;
		}
		//3 ���Ҷӳ�����
		net::S_CLIENT_BASE* leader_c = __TcpServer->client(leader_user->tmp.server_connectid, leader_user->tmp.server_clientid);
		if (leader_c == nullptr) return;

		S_TEAM_REQUEST data2;
		data2.memid = leader_mate->memid;
		data2.userindex = leader_mate->userindex;
		data2.user_connectid = leader_user->tmp.user_connectid;
		data2.otherindex = data.userindex;
		data2.othermemid = data.memid;

		char nick[USER_MAX_NICK];
		memset(nick, 0, USER_MAX_NICK);
		memcpy(nick, user->role.nick, USER_MAX_NICK);

		//xxxx ��ӳ������������
		__TcpServer->begin(leader_c->ID, CMD_7060);
		__TcpServer->sss(leader_c->ID, (u16)0);
		__TcpServer->sss(leader_c->ID, &data2, sizeof(S_TEAM_REQUEST));
		__TcpServer->sss(leader_c->ID, nick, USER_MAX_NICK);
		__TcpServer->end(leader_c->ID);
	}
	//7070 ������� �ӳ�ͬ�����
	void onTeamRequest_Join(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_TEAM_LEADER data;
		ts->read(c->ID, &data, sizeof(S_TEAM_LEADER));

		//1��û���ҵ�����
		S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7070, 7001, &data, sizeof(S_TEAM_LEADER));
			return;
		}
		//2 ���Ҽ�����
		S_USER_TEAM* join_user = __UserManager->findUser(data.joinindex);
		if (join_user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7070, 7002, &data, sizeof(S_TEAM_LEADER));
			return;
		}
		//2����֤�Ƿ��ڸ����ڣ������ڲ���������κβ��� ��;�뿪����=�˳�����
		bool iscopy = script::isCopyMap(user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7010, 7003, &data, sizeof(S_TEAM_LEADER));
			return;
		}
		iscopy = script::isCopyMap(join_user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7010, 7004, &data, sizeof(S_TEAM_LEADER));
			return;
		}
		//3 ���Ҽ��������� ���ط�����
		net::S_CLIENT_BASE* join_c = __TcpServer->client(join_user->tmp.server_connectid, join_user->tmp.server_clientid);
		if (join_c == nullptr)
		{
			sendErrInfo(c->ID, CMD_7070, 7005, &data, sizeof(S_TEAM_LEADER));
			return;
		}
		//4 �������Ѿ��ж�����
		if (join_user->tmp.temp_TeamIndex >= C_TEAM_START_POS)
		{
			sendErrInfo(c->ID, CMD_7070, 7006, &data, sizeof(S_TEAM_LEADER));
			return;
		}
		//5�����ҷ���
		S_TEAM_BASE * team = __AppTeam->findTeam(user->tmp.temp_TeamIndex);
		if (team == nullptr || !team->isused)
		{
			sendErrInfo(c->ID, CMD_7070, 7007, &data, sizeof(S_TEAM_LEADER));
			return;
		}
		//6�����Ҷ���
		S_TEAM_MATE * mate = team->findMate(user->tmp.temp_MateIndex);
		if (mate == nullptr || !mate->isleader)
		{
			sendErrInfo(c->ID, CMD_7070, 7008, &data, sizeof(S_TEAM_LEADER));
			return;
		}
		//7��Ѱ�ҿ�λ
		S_TEAM_MATE * join_mate = team->findEmptyMate();
		if (join_mate == nullptr)
		{
			sendErrInfo(c->ID, CMD_7070, 7009, &data, sizeof(S_TEAM_LEADER));
			return;
		}

		//���ö�������
		join_user->tmp.setTeamIndex(team->index, join_mate->mateindex);
		team->setNewUserTeam(data.joinindex, join_mate->mateindex, false);

		//1��֪ͨ�Լ�������� 
		nf_GateServer_TeamSelf(join_user, join_c->ID, false);
		bc_GateServer_TeamJoin(join_c, join_user, team);

		//2��֪ͨ��Ϸ������
		nf_GameServer_TeamSelf(join_user);
		bc_GameServer_TeamData(team->index);

		//���ö�������
		LOG_MESSAGE("onTeamRequest [%s]�����˶��� %d:%d\n", join_user->role.nick, team->index, join_mate->mateindex);

	}
	//7080 �ӳ��ܾ�
	void onTeamRequest_Refuse(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		//S_TEAM_LEADER data;
		//ts->read(c->ID, &data, sizeof(S_TEAM_LEADER));

		////1��û���ҵ�����
		//S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		//if (user == nullptr)
		//{
		//	sendErrInfo(c->ID, CMD_7080, 2001, &data, sizeof(S_TEAM_LEADER));
		//	return;
		//}
		////2 ���Ҽ�����
		//S_USER_TEAM* join_user = __UserManager->findUser(data.joinindex);
		//if (join_user == nullptr)
		//{
		//	sendErrInfo(c->ID, CMD_7080, 2002, &data, sizeof(S_TEAM_LEADER));
		//	return;
		//}
		////3 ���Ҽ��������� ���ط�����
		//net::S_CLIENT_BASE* join_c = __TcpServer->client(join_user->tmp.server_connectid, join_user->tmp.server_clientid);
		//if (join_c == nullptr)
		//{
		//	sendErrInfo(c->ID, CMD_7080, 2003, &data, sizeof(S_TEAM_LEADER));
		//	return;
		//}


		//char nick[USER_MAX_NICK];
		//memset(nick, 0, USER_MAX_NICK);
		//memcpy(nick, user->role.nick, USER_MAX_NICK);


		//S_TEAM_LEADER data2;
		//data2.user_connectid = join_user->tmp.server_connectid;
		//data2.memid = join_user->mem.id;
		//data2.userindex = data.joinindex;
		////�ӳ� �ܾ�������������
		//__TcpServer->begin(join_c->ID, CMD_7080);
		//__TcpServer->sss(join_c->ID, (u16)0);
		//__TcpServer->sss(join_c->ID, &data2,sizeof(S_TEAM_LEADER));
		//__TcpServer->sss(join_c->ID, nick, USER_MAX_NICK);
		//__TcpServer->end(join_c->ID);
	}
	//7090 ת�öӳ�
	void onTeamNewLeader(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_TEAM_NEWLEADER data;
		ts->read(c->ID, &data, sizeof(S_TEAM_NEWLEADER));

		//1��û���ҵ�����
		S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7090, 7001, &data, sizeof(S_TEAM_NEWLEADER));
			return;
		}
		bool iscopy = script::isCopyMap(user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7010, 7002, &data, sizeof(S_TEAM_NEWLEADER));
			return;
		}
		//2�����ҷ���
		S_TEAM_BASE* team = __AppTeam->findTeam(user->tmp.temp_TeamIndex);
		if (team == nullptr || !team->isused)
		{
			sendErrInfo(c->ID, CMD_7090, 7003, &data, sizeof(S_TEAM_NEWLEADER));
			return;
		}
		//3�����Ҷ��� ��֤�ӳ�
		S_TEAM_MATE* mate = team->findMate(user->tmp.temp_MateIndex);
		if (mate == nullptr || !mate->isleader || !mate->isused)
		{
			sendErrInfo(c->ID, CMD_7090, 7004, &data, sizeof(S_TEAM_NEWLEADER));
			return;
		}
		//4��Ѱ���¶ӳ�
		S_TEAM_MATE* new_mate = team->setNewLeader(data.mateindex);
		if (new_mate == nullptr || !new_mate->isused || !new_mate->isleader)
		{
			sendErrInfo(c->ID, CMD_7090, 7005, &data, sizeof(S_TEAM_NEWLEADER));
			return;
		}
		//5���������
		S_USER_TEAM* new_user = __UserManager->findUser(new_mate->userindex, new_mate->memid);
		if (new_user == nullptr)
		{
			sendErrInfo(c->ID, CMD_7090, 7006, &data, sizeof(S_TEAM_NEWLEADER));
			return;
		}
		iscopy = script::isCopyMap(new_user->status.mapid);
		if (iscopy)
		{
			sendErrInfo(c->ID, CMD_7090, 7007, &data, sizeof(S_TEAM_NEWLEADER));
			return;
		}
		//3 �����¶ӳ����� ���ط�����
		net::S_CLIENT_BASE* new_c = __TcpServer->client(new_user->tmp.server_connectid, new_user->tmp.server_clientid);
		if (new_c == nullptr)
		{
			sendErrInfo(c->ID, CMD_7090, 7008, &data, sizeof(S_TEAM_NEWLEADER));
			return;
		}
		mate->isleader = false;
		//�㲥����ҳ�Ϊ�ӳ�
		bc_GateServer_TeamLeader(team->index, new_mate->mateindex);
		bc_GameServer_TeamData(team->index);
	}

	bool AppTeam::onServerCommand(net::ITCPServer * ts, net::S_CLIENT_BASE * c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppLogin err...line:%d \n", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE && c->clientType != func::S_TYPE_GAME) return false;

		switch (cmd)
		{
		case CMD_7000:onTeamCreate(ts, c);            break;//�������� ok 
		case CMD_7010:onTeamPlease(ts, c);            break;//������� ok
		case CMD_7020:onTeamJoin(ts, c);              break;//ͬ�������� ok
		//case CMD_7030:onTeamRefuse(ts, c);            break;//�ܾ��������
		case CMD_7040:onTeamLeave(ts, c);             break;//�����뿪 ok
		case CMD_7050:onTeamOut(ts, c);               break;//�ӳ�����
		case CMD_7060:onTeamRequest(ts, c);           break;//���������� ok
		case CMD_7070:onTeamRequest_Join(ts, c);      break;//������� �ӳ�ͬ�� ok
		//case CMD_7080:onTeamRequest_Refuse(ts, c);    break;//������� �ӳ��ܾ�
		case CMD_7090:onTeamNewLeader(ts, c);         break;//�ӳ�ת��
		case CMD_7300:onStartCopy(ts, c);             break;//������ʼ
		case CMD_7400:onLeaveCopy(ts, c);             break;//�뿪����
		}
		return true;
	}

	//*****************************************************************
	//*****************************************************************
	//*****************************************************************
	


}