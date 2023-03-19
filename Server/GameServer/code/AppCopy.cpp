
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
	//初始化
	void AppCopy::onInit()
	{
		//1 初始化 判断有无副本地图 是否开启更新
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
		//2 开辟副本空间（只要有副本地图就开，没有就不开）
		__Rooms = new HashArray<S_COPY_ROOM>(MAX_LAYER_COUNT);
		//3 初始化
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

	//更新副本时间 100毫秒检查一次
	void AppCopy::onUpdate()
	{
		if (!isUpdate) return;
		//50毫秒检查一次
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

			//1、检查单人副本
			if (room->maptype == EMT_COPY_ONE)
			{
				//玩家数据为NULL 说明 已经掉线离开
				auto user = __UserManager->findUser(room->userindex, room->memid);
				if (user == nullptr)
				{
					LOG_MESSAGE("update:副本结束 玩家数据=NULL...index:%d/%lld \n", room->userindex, room->memid);

					AppCopy::clearCopy(room);
				}
			}
			//2、检查多人副本
			else if (room->maptype == EMT_COPY_MORE)
			{
				auto team = __AppTeam->findTeam(room->teamindex);
				if (team == nullptr) return;

				//只要有一个人在副本里面 副本就会存在 
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
					LOG_MESSAGE("update:副本结束 队伍没人了...line:%d \n", __LINE__);

					AppCopy::clearCopy(room);
					continue;
				}
			}


			//3、检查剩余时间
			room->limitime -= v;
			room->temp_time = global_gametime;

			//LOG_MSG("副本剩余时间...time:%d  %d  %d\n", room->limitime,v, value);
			if (room->limitime > 0) continue;

			LOG_MESSAGE( "update:副本结束 时间到了...line:%d \n", __LINE__);

			
			__RobotManager->clearRobot_Copy(room->mapid, i);
			AppDrop::clearDrop(room->mapid, i);
			TS_Broadcast::bc_ResetCopy(room, 2);
			room->reset();
		}
	}

	//寻找一个空闲的房间 下标从1 开始 
	//默认初始化为0 或者-1 
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
		//2、验证地图脚本
		auto cur_map = script::findScript_Map(user->role.base.status.mapid);
		auto copy_map = script::findScript_Map(data.mapid);
		if (cur_map == nullptr || copy_map == nullptr) return 3003;
		
		//3、验证当前地图是不是公用地图 不能在副本内传到副本
		if (cur_map->maptype != EMT_PUBLIC_MAP) return 3004;
		
		if (cmd == CMD_7200)
		{
			//4、不是单人副本
			if (copy_map->maptype != EMT_COPY_ONE) return 3005;
			//5、单人副本不能有队伍
			if (user->tmp.temp_TeamIndex > 0) return 3006;
		}
		else if (cmd == CMD_7300)
		{
			//4、不是多人人副本
			if (copy_map->maptype != EMT_COPY_MORE) return 3007;
			//5 验证队伍
			auto team = __AppTeam->findTeam(user->tmp.temp_TeamIndex);
			if (team == nullptr) return 3008;
			//6 验证队伍成员
			auto mate = team->findMate(user->tmp.temp_MateIndex);
			if (mate == nullptr) return 3009;
			//7 验证是不是队长
			if (mate->isleader == false) return 3010;

		}
		//8、等级限制
		if (user->role.base.exp.level < copy_map->copy_level) return 3011;
		//9、验证当前是否在副本中?
		if (user->tmp.temp_CopyIndex > 0) return 3012;

		return 0;
	}

	//检查玩家
	S_USER_BASE* checkCopyUser(S_CMD_COPY& data,s32 connectid,u16 cmd, u16 childcmd)
	{
		S_USER_BASE* user = nullptr;
		if (childcmd == 1) //加载角色角色数据
		{
			//1、验证玩家数据有效性 该地图已经有玩家数据 错误
			user = __UserManager->findUser(data.userindex);
			if (user != nullptr)
			{
				sendErrInfo(__TcpServer, connectid, cmd, 3021, &data, sizeof(S_CMD_COPY));
				return nullptr;
			}
		}
		else
		{
			//1、验证玩家数据有效性 必须是唯一索引+账号ID
			user = __UserManager->findUser(data.userindex, data.memid);
			if (user == nullptr)
			{
				sendErrInfo(__TcpServer, connectid, cmd, 3001, &data, sizeof(S_CMD_COPY));
				return nullptr;
			}
		}
		return user;
	}
	//7200 开启单人副本
	//7300 开启多人副本
	void onStartCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c,const u16 cmd)
	{
		u16 childcmd = 0;
		S_CMD_COPY data;
		ts->read(c->ID, childcmd);
		ts->read(c->ID, &data, sizeof(S_CMD_COPY));

		LOG_MESSAGE("copy start cmd:%d  %d go mapid:%d \n", cmd, childcmd,data.mapid);
		//2 开启副本加载角色数据
		if (childcmd == 1)
		{
			//1、验证玩家
			S_USER_BASE* user = checkCopyUser(data, c->ID, cmd, childcmd);
			if (user != nullptr) return;

			__UserManager->onPushLoadRole(c, &data,data.userindex, data.mapid, cmd,childcmd);
			return;
		}

		//1、验证玩家
		S_USER_BASE* user = checkCopyUser(data, c->ID, cmd, childcmd);
		if (user == nullptr) return;

		//3、验证开始副本
		s32 err = checkStartCopy(user,data, cmd);
		if(err != 0)
		{
			sendErrInfo(ts, c->ID, cmd, err, &data, sizeof(S_CMD_COPY));
			return;
		}

		//4、如果是多人副本 必须要判断当前组队服务器连接情况？
		if (cmd == CMD_7300)
		{
			if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE)
			{
				sendErrInfo(ts, c->ID, 3031, err, &data, sizeof(S_CMD_COPY));
				return;
			}
		}

		//进入副本之前 先保存下当前所在地图ID和坐标 用于退出副本还原地图和坐标
		user->role.base.status.copyMapid(false);

		//4、获取需要去副本地图所在的服务器ID，要切换服务器了返回的就是0
		u32 serverid = share::findGameServerID(share::__ServerLine, data.mapid);

		//要去的服务器就是当前所在服务器 就无须切换
		if (serverid == func::__ServerInfo->ID)
		{
			//房间已满
			S_COPY_ROOM* room = __AppCopy->findEmpty();
			if (room == nullptr)
			{
				sendErrInfo(ts, c->ID, cmd, 3014, &data, sizeof(S_CMD_COPY));
				return;
			}

			//切换地图
			//进入副本之前 先保存下当前所在地图ID和坐标 用于退出副本还原地图和坐标
			//获取房间ID和队伍ID
			data.roomindex = room->index;
			data.teamindex = user->tmp.temp_TeamIndex;
			
			//生成出生点坐标
			user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_NO);
			user->role.base.status.mapid = data.mapid;
			user->initBornPos(data.mapid);

			//设置副本 开始刷怪
			AppCopy::startCopy(room, user, data.mapid);

			//通知组队服务器 玩家切换地图
			TS_Broadcast::team_ChangeMap(user, CMD_901);
			//通知组队服务器 副本开启了 然后广播给各个玩家
			if(cmd == CMD_7300) TS_Broadcast::team_StartCopy(data, cmd);

			//返回切换地图成功
			TS_Broadcast::do_ChangeMap(user, &data, sizeof(S_CMD_COPY),c->ID, cmd);
			LOG_MESSAGE("copy changeMap success...%d cmapid:%d %d/%d/%d \n", data.mapid, 
				user->role.base.status.c_mapid, user->role.base.status.c_pos.x,
				user->role.base.status.c_pos.y, user->role.base.status.c_pos.z);
			return;
		}

		LOG_MESSAGE("copy changeMap start.....index:%d %lld %d \n", data.userindex, data.memid, data.mapid);
		
		user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_YES);
		//通知DB服务器 开始切换地图  
		TS_Broadcast::db_ChangeMap(c, &data, data.userindex, share::__ServerLine, data.mapid, cmd, 3000);
	}
	//********************************************************************
	//********************************************************************
	//离开副本 
	s32 checkLeaveCopy(S_USER_BASE* user)
	{
		//1、验证地图脚本
		auto cur_map = script::findScript_Map(user->role.base.status.mapid); //当前所在地图
		auto go_map = script::findScript_Map(user->role.base.status.c_mapid);//退出副本要去的地图
		auto map = __World->getMap(user->role.base.status.c_mapid); //要去的地图数据
		if (cur_map == nullptr || go_map == nullptr) return 3003;
		//2、验证当前地图不是副本 ？
		if (cur_map->maptype != EMT_COPY_ONE && cur_map->maptype != EMT_COPY_MORE) return 3004;
		//3、验证要去的地图是不是公用地图？
		if (go_map->maptype != EMT_PUBLIC_MAP) return 3005;
		return 0;
	}
	//7400 离开副本 = 切换地图
	void onLeaveCopy(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u16 childcmd = 0;
		S_CMD_COPY data;
		ts->read(c->ID, childcmd);
		ts->read(c->ID, &data, sizeof(S_CMD_COPY));


		//离开副本 切换地图加载角色数据
		if (childcmd == 1)
		{
			S_USER_BASE* user = checkCopyUser(data, c->ID, CMD_7400, childcmd);
			if (user != nullptr) return;

			__UserManager->onPushLoadRole(c, &data, data.userindex, data.mapid,CMD_7400, childcmd);
			return;
		}
		
		S_USER_BASE* user = checkCopyUser(data, c->ID, CMD_7400, childcmd);
		if (user == nullptr) return;
		//2、验证错误
		s32 err = checkLeaveCopy(user);
		if (err != 0)
		{
			sendErrInfo(ts, c->ID, CMD_7400, err, &data, sizeof(S_CMD_COPY));
			return;
		}
		data.mapid = user->role.base.status.c_mapid;
		data.roomindex = user->tmp.temp_CopyIndex;


		//8、获取需要返回地图所在的服务器ID
		//要去的服务器就是当前所在服务器 就无须切换
		u32 serverid = share::findGameServerID(share::__ServerLine, user->role.base.status.c_mapid);
		if (serverid == func::__ServerInfo->ID)
		{
			user->leaveCopy(true);//离开副本
			user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_NO);//切换地图 离开世界
			
			user->role.base.status.copyMapid(true);//恢复到原来的地图ID和坐标

			//通知组队服务器 玩家切换地图
			TS_Broadcast::team_ChangeMap(user, CMD_901);
			//通知网关切换地图
			TS_Broadcast::do_ChangeMap(user, &data, sizeof(S_CMD_COPY), c->ID, CMD_7400);
			return;
		}


		LOG_MESSAGE("leaveCopy changeMap start.....index:%d %lld %d \n", data.userindex, data.memid, data.mapid);

		user->leaveCopy(true);//离开副本
		user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_YES);
		//通知DB 开始切换地图
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
		case CMD_7200://单人副本
		case CMD_7300://多人副本
		case CMD_7301://加入多人副本
			onStartCopy(ts, c, cmd);
			break;
		case CMD_7400://离开副本
			onLeaveCopy(ts, c);
			break;
		}
		return false;
	}
}


