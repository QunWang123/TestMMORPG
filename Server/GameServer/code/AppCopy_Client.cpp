#include "AppCopy.h"
#include  "CommandData2.h"
#include "WorldMap.h"
#include "WorldBC.h"

#include "WorldTools.h"
#include "WorldScript.h"
#include  "UserManager.h"
#include  "AppTeam.h"
#include  "AppDrop.h"
#include  "RobotManager.h"
#include "ShareFunction.h"
namespace app
{
	//������
	void AppCopy::clearCopy(S_COPY_ROOM* room)
	{
		if (room == nullptr) return;
		if (room->state == ECRS_FREE) return;
		S_WORLD_MAP* map = __World->getMap(room->mapid);
		if (map == nullptr) return;
		auto copy_map = script::findScript_Map(room->mapid);
		if (copy_map == nullptr) return;
		if (copy_map->maptype == EMT_PUBLIC_MAP) return;

		//����㼶����
		map->Layer.clearLayerType(room->index);
		//�������
		__RobotManager->clearRobot_Copy(room->mapid, room->index);
		//�������
		AppDrop::clearDrop(room->mapid, room->index);
		//������
		room->reset();
	}

	//��ʼ���� ˢ��
	void AppCopy::startCopy(S_COPY_ROOM* room, S_USER_BASE* user, const u32 mapid)
	{
		if (room == nullptr) return;
		if (user == nullptr) return;

		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return;
		auto copy_map = script::findScript_Map(mapid);
		if (copy_map == nullptr) return;

		//������Ϊ ����ID
		room->state = ECRS_GAMING;
		room->mapid = copy_map->mapid;
		room->maptype = copy_map->maptype;
		room->limitime = copy_map->copy_limittime;
		room->userindex = user->tmp.userindex;
		room->memid = user->mem.id;
		room->teamindex = room->teamindex = user->tmp.temp_TeamIndex;
		room->temp_time = global_gametime;


		user->node.layer = room->index;
		user->tmp.temp_CopyIndex = room->index;

		//����һ���㼶���� ��Ҫ������������߽�ɫ ����ĸ�����Ϣ
		//������� ��ɫ ���� �ص���һ��
		map->Layer.setLayerType(room->index);

		//ˢ��
		__RobotManager->clearRobot_Copy(copy_map->mapid, user->node.layer);
		__RobotManager->createRobot_Copy(copy_map->mapid, user->node.layer);

		LOG_MESSAGE("��������:%d \n", room->limitime);

		////���˸�����¼�� ����Ĵ���ʱ�� ��Ҫ���� �������� ƾ�� 
		//if (room->maptype != EMT_COPY_MORE) return;
		//auto team = __AppTeam->findTeam(room->teamindex);
		//if (team == nullptr) return;
		//
		//room->temp_teamCreatetime = team->createtime;

	}
	//****************************************************************************
	//****************************************************************************
	//7200 �յ�DB���������صļ��ؽ�ɫ���ݻ����л���ͼOK����
	//7300 �յ�DB���������صļ��ؽ�ɫ���ݻ����л���ͼOK����
	//7400 �뿪���� 
	void onCMD_7200(net::ITCPClient* tc, const u16 cmd)
	{
		u16 childcmd = 0;
		S_LOAD_ROLE data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_LOAD_ROLE));

		S_CMD_COPY data2;
		data2.userindex = data.userindex;
		data2.memid = data.memid;
		data2.user_connectid = data.user_connectid;
		data2.mapid = data.mapid;

		// childcmd 0 ���ؽ�ɫ���� 3000 �л���ͼ
		if (childcmd == 3000)
		{
			LOG_MESSAGE("[db save success] onCMD_%d:%d... go mapid:%d [%d/%lld]\n", cmd, childcmd, data.mapid, data.userindex, data.memid);
		}
		else
		{
			LOG_MESSAGE("[db load role] onCMD_%d:%d... go mapid:%d [%d/%lld]\n", cmd, childcmd, data.mapid, data.userindex, data.memid);
		}

		auto c = __TcpServer->client(data.server_connectid, data.server_clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("onCMD_%d c == NULL...%d %d line:%d \n", cmd, data.server_connectid, data.server_clientid, __LINE__);
			return;
		}
		//�յ�DB�������ݳɹ���Ϣ ֪ͨgate��ʼ�л���ͼ
		if (childcmd == 3000)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3000, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//DDB���ش���
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, childcmd, &data2, sizeof(S_CMD_COPY));
			return;
		}

		//*******************************************************************
		//1����֤���������Ч��
		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user != NULL)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3020, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//2��������������
		user = __UserManager->popUser();
		user->reset();
		bool isok = __UserManager->insertUser(data.userindex, user);
		if (isok == false)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3021, &data2, sizeof(S_CMD_COPY));
			return;
		}
		auto room = __AppCopy->findEmpty();
		if (room == nullptr)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3022, &data, sizeof(S_CMD_COPY));
			return;
		}

		//1����ȡ����
		tc->read(&user->mem, sizeof(S_USER_MEMBER_DATA));
		tc->read(&user->role, sizeof(S_USER_ROLE));

		auto copy_map = script::findScript_Map(data.mapid);
		if (copy_map == nullptr)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3024, &data2, sizeof(S_CMD_COPY));
			return;
		}
		if (cmd == CMD_7300)
		{
			auto team = __AppTeam->findTeam(data.memid, user->tmp.temp_TeamIndex, user->tmp.temp_MateIndex);
			if (team == nullptr)
			{
				sendErrInfo(__TcpServer, c->ID, cmd, 3025, &data2, sizeof(S_CMD_COPY));
				return;
			}
		}
		//set data
		data2.teamindex = user->tmp.temp_TeamIndex;
		data2.roomindex = room->index;
		user->tmp.userindex = data.userindex;
		user->tmp.server_connectid = c->ID;
		user->tmp.server_clientid = c->clientID;
		user->tmp.user_connectid = data.user_connectid;
		user->tmp.temp_HeartTime = (int)time(NULL);


		switch (cmd)
		{
		case CMD_7200:
		case CMD_7300:
			//�л���ͼ�ɹ�
			user->changeMap(&data2, sizeof(S_CMD_COPY), c->ID, cmd, ECK_SUCCESS);
			user->role.base.status.mapid = data.mapid;
			user->initBornPos(data.mapid);
			AppCopy::startCopy(room, user, data.mapid);
			break;
		case CMD_7400:
			user->role.base.status.copyMapid(true);//�ָ���ԭ���ĵ�ͼID������
			//�л���ͼ�ɹ�
			user->changeMap(&data2, sizeof(S_CMD_COPY), c->ID, cmd, ECK_SUCCESS);
			break;
		}

		//֪ͨ��ӷ����� ����л���ͼ
		TS_Broadcast::team_ChangeMap(user, CMD_901);
		//֪ͨ��ӷ����� ���������� Ȼ��㲥���������
		if (cmd == CMD_7300) TS_Broadcast::team_StartCopy(data2, cmd);
		//֪ͨ���� ����л���ͼ
		TS_Broadcast::do_ChangeMap(user, &data2, sizeof(S_CMD_COPY), c->ID, cmd);


		LOG_MESSAGE("[db loadrole] .cmd:%d %d %d copy:%d team:%d/%d id:%d\n", cmd,user->node.index,user->node.layer,
			user->tmp.temp_CopyIndex, user->tmp.temp_TeamIndex, user->tmp.temp_MateIndex,data2.user_connectid);
	}
	//���ѽ��븱��
	void onCMD_7301(net::ITCPClient* tc, const u16 cmd)
	{
		u16 childcmd = 0;
		S_LOAD_ROLE data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_LOAD_ROLE));

		S_CMD_COPY data2;
		data2.userindex = data.userindex;
		data2.memid = data.memid;
		data2.user_connectid = data.user_connectid;
		data2.mapid = data.mapid;
		

		if (childcmd == 3000)
		{
			LOG_MESSAGE("[db save success] onCMD_%d:%d... go mapid:%d [%d/%lld]\n", cmd, childcmd, data.mapid, data.userindex, data.memid);
		}
		else
		{
			LOG_MESSAGE("[db load role] onCMD_%d:%d... go mapid:%d [%d/%lld]\n", cmd, childcmd, data.mapid, data.userindex, data.memid);
		}

		auto c = __TcpServer->client(data.server_connectid, data.server_clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("onCMD_%d c == NULL...%d %d line:%d \n", cmd, data.server_connectid, data.server_clientid, __LINE__);
			return;
		}
		//�յ�DB�������ݳɹ���Ϣ ֪ͨgate��ʼ�л���ͼ
		if (childcmd == 3000)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3000, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//DDB���ش���
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, childcmd, &data2, sizeof(S_CMD_COPY));
			return;
		}

		//*******************************************************************
		//1����֤���������Ч��
		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user != NULL)
		{
			LOG_MESSAGE("onCMD_%d err user != NULL...%d  line:%d \n", cmd, data.userindex, __LINE__);
			sendErrInfo(__TcpServer, c->ID, cmd, 3040, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//2��������������
		user = __UserManager->popUser();
		user->reset();
		bool isok = __UserManager->insertUser(data.userindex, user);
		if (isok == false)
		{
			LOG_MESSAGE("onCMD_%d err  isok=false...%d  line:%d \n", cmd, data.userindex, __LINE__);
			sendErrInfo(__TcpServer, c->ID, cmd, 3041, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//1����ȡ����
		tc->read(&user->mem, sizeof(S_USER_MEMBER_DATA));
		tc->read(&user->role, sizeof(S_USER_ROLE));

		auto copy_map = script::findScript_Map(data.mapid);
		if (copy_map == nullptr)
		{
			LOG_MESSAGE("onCMD_%d err  copy_map=NULL...%d  line:%d \n", cmd, data.userindex, __LINE__);
			sendErrInfo(__TcpServer, c->ID, cmd, 3042, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//�����Լ��Ķ��� ���ҷ���
		auto room = __AppCopy->findRoom(user->mem.id,user->tmp.temp_MateIndex);
		if(room == NULL)
		{
			LOG_MESSAGE("onCMD_7301 err  room=NULL...%lld  line:%d \n", user->mem.id, __LINE__);
			sendErrInfo(__TcpServer, c->ID, cmd, 3043, &data2, sizeof(S_CMD_COPY));
			return;
		}

		//set data
		user->node.layer = room->index;
		user->tmp.temp_TeamIndex = room->teamindex;
		user->tmp.temp_CopyIndex = room->index;
		user->tmp.userindex = data.userindex;
		user->tmp.server_connectid = c->ID;
		user->tmp.server_clientid = c->clientID;
		user->tmp.user_connectid = data.user_connectid;
		user->tmp.temp_HeartTime = (int)time(NULL);
		//�л���ͼ�ɹ�
		user->changeMap(&data2, sizeof(S_CMD_COPY), c->ID, cmd, ECK_SUCCESS);
		user->role.base.status.mapid = data.mapid;
		user->initBornPos(data.mapid);
		user->node.layer = room->index;
		
		//֪ͨ��ӷ����� ����л���ͼ
		TS_Broadcast::team_ChangeMap(user, CMD_901);
	
		//֪ͨ���� ����л���ͼ
		TS_Broadcast::do_ChangeMap(user, &data2, sizeof(S_CMD_COPY), c->ID, cmd);


		LOG_MESSAGE("[db loadrole] 7301  mem:%d/%lld layer:%d mapid:%d/%d copy:%d team:%d/%d\n",
			user->node.index,user->mem.id, user->node.layer,data2.mapid,data2.user_connectid,
			user->tmp.temp_CopyIndex, user->tmp.temp_TeamIndex, user->tmp.temp_MateIndex);
	}

	//7301 �յ���ӷ��������� ��ʼ���븱������Ϣ
	void onCMD_7301_Team(net::ITCPClient* tc)
	{
		S_CMD_COPY data;
		tc->read(&data, sizeof(S_CMD_COPY));

		//1����֤���
		S_USER_BASE* user = __UserManager->findUser(data.userindex,data.memid);
		if (user == nullptr) 
		{
			LOG_MESSAGE("[TeamServer] onCMD_7301 ueser == null...%d %lld line:%d\n", data.userindex, data.memid, __LINE__);
			return;
		}
		if (user->tmp.temp_TeamIndex != data.teamindex)
		{
			LOG_MESSAGE("[TeamServer] onCMD_7301 teamindex err...%d %d %d line:%d\n", data.userindex, user->tmp.temp_TeamIndex,data.teamindex, __LINE__);
			return;
		}
		auto c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
		if(c == nullptr)
		{
			LOG_MESSAGE("[TeamServer] onCMD_7301 c == null...%d %d %d line:%d\n", data.userindex, user->tmp.server_connectid, user->tmp.server_clientid, __LINE__);
			return;
		}
		//���븱��֮ǰ �ȱ����µ�ǰ���ڵ�ͼID������ �����˳�������ԭ��ͼ������
		user->role.base.status.copyMapid(false);
		//4����ȡ��Ҫȥ������ͼ���ڵķ�����ID
		u32 serverid = share::findGameServerID(share::__ServerLine, data.mapid);
		//Ҫȥ�ķ��������ǵ�ǰ���ڷ����� �������л�
		if (serverid == func::__ServerInfo->ID)
		{
			auto room = __AppCopy->findRoom(data.roomindex);
			if(room == nullptr)
			{
				LOG_MESSAGE("[TeamServer] onCMD_7301 room=null...%d %d line:%d\n", data.userindex, data.roomindex, __LINE__);
				return;
			}
			if (room->state != ECRS_GAMING)
			{
				LOG_MESSAGE("[TeamServer] onCMD_7301 state err...%d %d %d line:%d\n", data.userindex, data.roomindex,room->state, __LINE__);
				return;
			}
			if (room->teamindex != user->tmp.temp_TeamIndex)
			{
				LOG_MESSAGE("[TeamServer] onCMD_7301 state err...%d %d %d line:%d\n", data.userindex, data.roomindex, room->state, __LINE__);
				return;
			}
			//���ø��� ��ʼˢ��
			user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_NO);
			user->role.base.status.mapid = data.mapid;
			user->initBornPos(data.mapid);
			user->node.layer = room->index;
			user->tmp.temp_CopyIndex = room->index;
			data.user_connectid = user->tmp.user_connectid;
			
			//֪ͨ��ӷ����� ����л���ͼ
		    TS_Broadcast::team_ChangeMap(user, CMD_901);
			//�����л���ͼ�ɹ�
			TS_Broadcast::do_ChangeMap(user, &data, sizeof(S_CMD_COPY), c->ID, CMD_7300);
			LOG_MESSAGE("[TeamServer] onCMD_7301 copy changeMap success...%d:%lld mapid:%d %d\n", data.userindex, data.memid, data.mapid, c->ID);
			return;
		}
		LOG_MESSAGE("[TeamServer] onCMD_7301 copy changeMap start.....index:%d %lld %d \n", data.userindex, data.memid, data.mapid);

		user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_YES);
		//֪ͨDB������ ��ʼ�л���ͼ  
		TS_Broadcast::db_ChangeMap(c, &data, data.userindex, share::__ServerLine, data.mapid, CMD_7301, 3000);
	}
	bool AppCopy::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_DB && tc->getData()->serverType != func::S_TYPE_CENTER) return false;

		if (tc->getData()->serverType == func::S_TYPE_CENTER)
		{
			switch (cmd)
			{
			case CMD_7301://���븱��
				onCMD_7301_Team(tc);
				break;
			}
			return true;
		}
		switch (cmd)
		{
		case CMD_7200:
		case CMD_7300:
		case CMD_7400:
			onCMD_7200(tc, cmd); 
			break;
		case CMD_7301:
			onCMD_7301(tc, cmd);
			break;
		}
		return true;
	}
}