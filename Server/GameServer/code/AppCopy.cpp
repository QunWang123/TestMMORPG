
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
	AppCopy*  __AppCopy = nullptr;
	s32 temp_index = 0;
	s32 temp_checkTime = 0;

	AppCopy::AppCopy(){ onInit(); }
	AppCopy::~AppCopy(){}
	//��ʼ��
	void AppCopy::onInit()
	{
		//1 ��ʼ�� �ж����޸�����ͼ �Ƿ�������
		roomCount = MAX_LAYER_COUNT;
		isUpdate = false;
		for (int i = 0; i < MAX_MAP_ID; i++)
		{
			auto map = &__World->map[i];
			if (map->isUsed == false) continue;
			auto sc = script::findScript_Map(i);
			if (sc == nullptr) continue;

			if (sc->maptype == EMT_PUBLIC_MAP) continue;
			isUpdate = true;
			break;
		}
		//2 ���ٸ����ռ䣨ֻҪ�и�����ͼ�Ϳ���û�оͲ�����
		__Rooms = new HashArray<S_COPY_ROOM>(MAX_LAYER_COUNT);
		//3 ��ʼ��
		reset();
	}

	void  AppCopy::reset()
	{
		for (u32 i = 0; i < MAX_LAYER_COUNT; i++)
		{
			S_COPY_ROOM* room = findRoom(i);
			if (room == nullptr) continue;
			room->reset();
		}
	}

	S_COPY_ROOM* AppCopy::findRoom(const s64 memid,  s8& mateindex)
	{
		for (s32 i = 0; i < __Rooms->length; i++)
		{
			auto room = findRoom(i);
			if (room == NULL) continue;
			if (room->state == ECRS_FREE) continue;

			auto team = __AppTeam->findTeam(room->teamindex);
			if (team == NULL) continue;
			for (u32 j = 0; j < C_TEAM_PLAYERMAX; j++)
			{
				auto mate = team->findMate(j);
				if (mate == NULL) continue;
				if (mate->memid == memid && memid > 0)
				{
					mateindex = j;
					return room;
				}
			}
		}
		return NULL;
	}

	//���¸���ʱ�� 100������һ��
	void AppCopy::onUpdate()
	{
		if (!isUpdate) return;
		//50������һ��
		s32 value = global_gametime - temp_checkTime;
		if (value < 50) return;
		temp_checkTime = global_gametime;

		
		s32 min = temp_index;
		temp_index += 100;
		s32  max = temp_index;

		if (max >= MAX_LAYER_COUNT)  max = MAX_LAYER_COUNT;
		if (temp_index >= MAX_LAYER_COUNT) temp_index = 0;
		
		for (u32 i = min; i < max; i++)
		{
			S_COPY_ROOM* room = findRoom(i);
			if (room == nullptr) continue;
			if (room->state == ECRS_FREE) continue;

			s32 v = global_gametime - room->temp_time;
			if (room->temp_time <= 0) v = 0;

			//1����鵥�˸���
			if (room->maptype == EMT_COPY_ONE)
			{
				//�������ΪNULL ˵�� �Ѿ������뿪
				auto user = __UserManager->findUser(room->userindex, room->memid);
				if (user == nullptr)
				{
					LOG_MESSAGE("update:�������� �������=NULL...index:%d/%lld \n", room->userindex, room->memid);

					AppCopy::clearCopy(room);
				}
			}
			//2�������˸���
			else if (room->maptype == EMT_COPY_MORE)
			{
				auto team = __AppTeam->findTeam(room->teamindex);
				if (team == nullptr) return;

				//ֻҪ��һ�����ڸ������� �����ͻ���� 
				bool isclear = true;
				for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
				{
					auto mate = team->findMate(i);
					if (mate == nullptr) continue;
					if (mate->isT() == false) continue;
					auto user = __UserManager->findUser(mate->userindex, mate->memid);
					if (user == nullptr) continue;
					if (user->role.base.status.mapid != room->mapid || user->node.layer != room->index) continue;
					if (user->tmp.temp_CopyIndex != room->index) continue;

					isclear = false;
					break;
				}
				if (isclear)
				{
					LOG_MESSAGE("update:�������� ����û����...line:%d \n", __LINE__);

					AppCopy::clearCopy(room);
					continue;
				}
			}


			//3�����ʣ��ʱ��
			room->limitime -= v;
			room->temp_time = global_gametime;

			//LOG_MSG("����ʣ��ʱ��...time:%d  %d  %d\n", room->limitime,v, value);
			if (room->limitime > 0) continue;

			LOG_MESSAGE( "update:�������� ʱ�䵽��...line:%d \n", __LINE__);

			
			__RobotManager->clearRobot_Copy(room->mapid, i);
			AppDrop::clearDrop(room->mapid, i);
			TS_Broadcast::bc_ResetCopy(room, 2);
			room->reset();
		}
	}

	//Ѱ��һ�����еķ��� �±��1 ��ʼ 
	//Ĭ�ϳ�ʼ��Ϊ0 ����-1 
	S_COPY_ROOM* AppCopy::findEmpty()
	{
		for (u32 i = 1; i < MAX_LAYER_COUNT; i++)
		{
			S_COPY_ROOM* room = findRoom(i);
			if (room == nullptr) continue;
			if (room->state != ECRS_FREE) continue;
			room->index = i;
			return room;
		}
		return nullptr;
	}

	s32 checkStartCopy(S_USER_BASE* user, S_CMD_COPY& data, const u16 cmd)
	{
		//2����֤��ͼ�ű�
		auto cur_map = script::findScript_Map(user->role.base.status.mapid);
		auto copy_map = script::findScript_Map(data.mapid);
		if (cur_map == nullptr || copy_map == nullptr) return 3003;
		
		//3����֤��ǰ��ͼ�ǲ��ǹ��õ�ͼ �����ڸ����ڴ�������
		if (cur_map->maptype != EMT_PUBLIC_MAP) return 3004;
		
		if (cmd == CMD_7200)
		{
			//4�����ǵ��˸���
			if (copy_map->maptype != EMT_COPY_ONE) return 3005;
			//5�����˸��������ж���
			if (user->tmp.temp_TeamIndex > 0) return 3006;
		}
		else if (cmd == CMD_7300)
		{
			//4�����Ƕ����˸���
			if (copy_map->maptype != EMT_COPY_MORE) return 3007;
			//5 ��֤����
			auto team = __AppTeam->findTeam(user->tmp.temp_TeamIndex);
			if (team == nullptr) return 3008;
			//6 ��֤�����Ա
			auto mate = team->findMate(user->tmp.temp_MateIndex);
			if (mate == nullptr) return 3009;
			//7 ��֤�ǲ��Ƕӳ�
			if (mate->isleader == false) return 3010;

		}
		//8���ȼ�����
		if (user->role.base.exp.level < copy_map->copy_level) return 3011;
		//9����֤��ǰ�Ƿ��ڸ�����?
		if (user->tmp.temp_CopyIndex > 0) return 3012;

		return 0;
	}

	//������
	S_USER_BASE* checkCopyUser(S_CMD_COPY& data,s32 connectid,u16 cmd, u16 childcmd)
	{
		S_USER_BASE* user = nullptr;
		if (childcmd == 1) //���ؽ�ɫ��ɫ����
		{
			//1����֤���������Ч�� �õ�ͼ�Ѿ���������� ����
			user = __UserManager->findUser(data.userindex);
			if (user != nullptr)
			{
				sendErrInfo(__TcpServer, connectid, cmd, 3021, &data, sizeof(S_CMD_COPY));
				return nullptr;
			}
		}
		else
		{
			//1����֤���������Ч�� ������Ψһ����+�˺�ID
			user = __UserManager->findUser(data.userindex, data.memid);
			if (user == nullptr)
			{
				sendErrInfo(__TcpServer, connectid, cmd, 3001, &data, sizeof(S_CMD_COPY));
				return nullptr;
			}
		}
		return user;
	}
	//7200 �������˸���
	//7300 �������˸���
	void onStartCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c,const u16 cmd)
	{
		u16 childcmd = 0;
		S_CMD_COPY data;
		ts->read(c->ID, childcmd);
		ts->read(c->ID, &data, sizeof(S_CMD_COPY));

		LOG_MESSAGE("copy start cmd:%d  %d go mapid:%d \n", cmd, childcmd,data.mapid);
		//2 �����������ؽ�ɫ����
		if (childcmd == 1)
		{
			//1����֤���
			S_USER_BASE* user = checkCopyUser(data, c->ID, cmd, childcmd);
			if (user != nullptr) return;

			__UserManager->onPushLoadRole(c, &data,data.userindex, data.mapid, cmd,childcmd);
			return;
		}

		//1����֤���
		S_USER_BASE* user = checkCopyUser(data, c->ID, cmd, childcmd);
		if (user == nullptr) return;

		//3����֤��ʼ����
		s32 err = checkStartCopy(user,data, cmd);
		if(err != 0)
		{
			sendErrInfo(ts, c->ID, cmd, err, &data, sizeof(S_CMD_COPY));
			return;
		}

		//4������Ƕ��˸��� ����Ҫ�жϵ�ǰ��ӷ��������������
		if (cmd == CMD_7300)
		{
			if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE)
			{
				sendErrInfo(ts, c->ID, 3031, err, &data, sizeof(S_CMD_COPY));
				return;
			}
		}

		//���븱��֮ǰ �ȱ����µ�ǰ���ڵ�ͼID������ �����˳�������ԭ��ͼ������
		user->role.base.status.copyMapid(false);

		//4����ȡ��Ҫȥ������ͼ���ڵķ�����ID��Ҫ�л��������˷��صľ���0
		u32 serverid = share::findGameServerID(share::__ServerLine, data.mapid);

		//Ҫȥ�ķ��������ǵ�ǰ���ڷ����� �������л�
		if (serverid == func::__ServerInfo->ID)
		{
			//��������
			S_COPY_ROOM* room = __AppCopy->findEmpty();
			if (room == nullptr)
			{
				sendErrInfo(ts, c->ID, cmd, 3014, &data, sizeof(S_CMD_COPY));
				return;
			}

			//�л���ͼ
			//���븱��֮ǰ �ȱ����µ�ǰ���ڵ�ͼID������ �����˳�������ԭ��ͼ������
			//��ȡ����ID�Ͷ���ID
			data.roomindex = room->index;
			data.teamindex = user->tmp.temp_TeamIndex;
			
			//���ɳ���������
			user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_NO);
			user->role.base.status.mapid = data.mapid;
			user->initBornPos(data.mapid);

			//���ø��� ��ʼˢ��
			AppCopy::startCopy(room, user, data.mapid);

			//֪ͨ��ӷ����� ����л���ͼ
			TS_Broadcast::team_ChangeMap(user, CMD_901);
			//֪ͨ��ӷ����� ���������� Ȼ��㲥���������
			if(cmd == CMD_7300) TS_Broadcast::team_StartCopy(data, cmd);

			//�����л���ͼ�ɹ�
			TS_Broadcast::do_ChangeMap(user, &data, sizeof(S_CMD_COPY),c->ID, cmd);
			LOG_MESSAGE("copy changeMap success...%d cmapid:%d %d/%d/%d \n", data.mapid, 
				user->role.base.status.c_mapid, user->role.base.status.c_pos.x,
				user->role.base.status.c_pos.y, user->role.base.status.c_pos.z);
			return;
		}

		LOG_MESSAGE("copy changeMap start.....index:%d %lld %d \n", data.userindex, data.memid, data.mapid);
		
		user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_YES);
		//֪ͨDB������ ��ʼ�л���ͼ  
		TS_Broadcast::db_ChangeMap(c, &data, data.userindex, share::__ServerLine, data.mapid, cmd, 3000);
	}
	//********************************************************************
	//********************************************************************
	//�뿪���� 
	s32 checkLeaveCopy(S_USER_BASE* user)
	{
		//1����֤��ͼ�ű�
		auto cur_map = script::findScript_Map(user->role.base.status.mapid); //��ǰ���ڵ�ͼ
		auto go_map = script::findScript_Map(user->role.base.status.c_mapid);//�˳�����Ҫȥ�ĵ�ͼ
		auto map = __World->getMap(user->role.base.status.c_mapid); //Ҫȥ�ĵ�ͼ����
		if (cur_map == nullptr || go_map == nullptr) return 3003;
		//2����֤��ǰ��ͼ���Ǹ��� ��
		if (cur_map->maptype != EMT_COPY_ONE && cur_map->maptype != EMT_COPY_MORE) return 3004;
		//3����֤Ҫȥ�ĵ�ͼ�ǲ��ǹ��õ�ͼ��
		if (go_map->maptype != EMT_PUBLIC_MAP) return 3005;
		return 0;
	}
	//7400 �뿪���� = �л���ͼ
	void onLeaveCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u16 childcmd = 0;
		S_CMD_COPY data;
		ts->read(c->ID, childcmd);
		ts->read(c->ID, &data, sizeof(S_CMD_COPY));


		//�뿪���� �л���ͼ���ؽ�ɫ����
		if (childcmd == 1)
		{
			S_USER_BASE* user = checkCopyUser(data, c->ID, CMD_7400, childcmd);
			if (user != nullptr) return;

			__UserManager->onPushLoadRole(c, &data, data.userindex, data.mapid,CMD_7400, childcmd);
			return;
		}
		
		S_USER_BASE* user = checkCopyUser(data, c->ID, CMD_7400, childcmd);
		if (user == nullptr) return;
		//2����֤����
		s32 err = checkLeaveCopy(user);
		if (err != 0)
		{
			sendErrInfo(ts, c->ID, CMD_7400, err, &data, sizeof(S_CMD_COPY));
			return;
		}
		data.mapid = user->role.base.status.c_mapid;
		data.roomindex = user->tmp.temp_CopyIndex;


		//8����ȡ��Ҫ���ص�ͼ���ڵķ�����ID
		//Ҫȥ�ķ��������ǵ�ǰ���ڷ����� �������л�
		u32 serverid = share::findGameServerID(share::__ServerLine, user->role.base.status.c_mapid);
		if (serverid == func::__ServerInfo->ID)
		{
			user->leaveCopy(true);//�뿪����
			user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_NO);//�л���ͼ �뿪����
			
			user->role.base.status.copyMapid(true);//�ָ���ԭ���ĵ�ͼID������

			//֪ͨ��ӷ����� ����л���ͼ
			TS_Broadcast::team_ChangeMap(user, CMD_901);
			//֪ͨ�����л���ͼ
			TS_Broadcast::do_ChangeMap(user, &data, sizeof(S_CMD_COPY), c->ID, CMD_7400);
			return;
		}


		LOG_MESSAGE("leaveCopy changeMap start.....index:%d %lld %d \n", data.userindex, data.memid, data.mapid);

		user->leaveCopy(true);//�뿪����
		user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_YES);
		//֪ͨDB ��ʼ�л���ͼ
		TS_Broadcast::db_ChangeMap(c, &data, data.userindex, share::__ServerLine, data.mapid, CMD_7400, 3000);
	}



	bool AppCopy::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppCopy err...line:%d \n", __LINE__);
			return false;
		}
		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_7200://���˸���
		case CMD_7300://���˸���
		case CMD_7301://������˸���
			onStartCopy(ts, c, cmd);
			break;
		case CMD_7400://�뿪����
			onLeaveCopy(ts, c);
			break;
		}
		return false;
	}
}


