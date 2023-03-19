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
	//清理副本
	void AppCopy::clearCopy(S_COPY_ROOM* room)
	{
		if (room == nullptr) return;
		if (room->state == ECRS_FREE) return;
		S_WORLD_MAP* map = __World->getMap(room->mapid);
		if (map == nullptr) return;
		auto copy_map = script::findScript_Map(room->mapid);
		if (copy_map == nullptr) return;
		if (copy_map->maptype == EMT_PUBLIC_MAP) return;

		//清理层级类型
		map->Layer.clearLayerType(room->index);
		//清理怪物
		__RobotManager->clearRobot_Copy(room->mapid, room->index);
		//清理道具
		AppDrop::clearDrop(room->mapid, room->index);
		//清理房间
		room->reset();
	}

	//开始副本 刷怪
	void AppCopy::startCopy(S_COPY_ROOM* room, S_USER_BASE* user, const u32 mapid)
	{
		if (room == nullptr) return;
		if (user == nullptr) return;

		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return;
		auto copy_map = script::findScript_Map(mapid);
		if (copy_map == nullptr) return;

		//层设置为 房间ID
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

		//设置一个层级数据 主要用于填充怪物或者角色 掉落的格子信息
		//避免怪物 角色 掉落 重叠在一起
		map->Layer.setLayerType(room->index);

		//刷怪
		__RobotManager->clearRobot_Copy(copy_map->mapid, user->node.layer);
		__RobotManager->createRobot_Copy(copy_map->mapid, user->node.layer);

		LOG_MESSAGE("开启副本:%d \n", room->limitime);

		////多人副本记录下 队伍的创建时间 主要用于 副本清理 凭据 
		//if (room->maptype != EMT_COPY_MORE) return;
		//auto team = __AppTeam->findTeam(room->teamindex);
		//if (team == nullptr) return;
		//
		//room->temp_teamCreatetime = team->createtime;

	}
	//****************************************************************************
	//****************************************************************************
	//7200 收到DB服务器返回的加载角色数据或者切换地图OK数据
	//7300 收到DB服务器返回的加载角色数据或者切换地图OK数据
	//7400 离开副本 
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

		// childcmd 0 加载角色数据 3000 切换地图
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
		//收到DB保存数据成功消息 通知gate开始切换地图
		if (childcmd == 3000)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3000, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//DDB返回错误
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, childcmd, &data2, sizeof(S_CMD_COPY));
			return;
		}

		//*******************************************************************
		//1、验证玩家数据有效性
		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user != NULL)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3020, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//2、添加新玩家数据
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

		//1、读取数据
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
			//切换地图成功
			user->changeMap(&data2, sizeof(S_CMD_COPY), c->ID, cmd, ECK_SUCCESS);
			user->role.base.status.mapid = data.mapid;
			user->initBornPos(data.mapid);
			AppCopy::startCopy(room, user, data.mapid);
			break;
		case CMD_7400:
			user->role.base.status.copyMapid(true);//恢复到原来的地图ID和坐标
			//切换地图成功
			user->changeMap(&data2, sizeof(S_CMD_COPY), c->ID, cmd, ECK_SUCCESS);
			break;
		}

		//通知组队服务器 玩家切换地图
		TS_Broadcast::team_ChangeMap(user, CMD_901);
		//通知组队服务器 副本开启了 然后广播给各个玩家
		if (cmd == CMD_7300) TS_Broadcast::team_StartCopy(data2, cmd);
		//通知网关 玩家切换地图
		TS_Broadcast::do_ChangeMap(user, &data2, sizeof(S_CMD_COPY), c->ID, cmd);


		LOG_MESSAGE("[db loadrole] .cmd:%d %d %d copy:%d team:%d/%d id:%d\n", cmd,user->node.index,user->node.layer,
			user->tmp.temp_CopyIndex, user->tmp.temp_TeamIndex, user->tmp.temp_MateIndex,data2.user_connectid);
	}
	//队友进入副本
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
		//收到DB保存数据成功消息 通知gate开始切换地图
		if (childcmd == 3000)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3000, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//DDB返回错误
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, childcmd, &data2, sizeof(S_CMD_COPY));
			return;
		}

		//*******************************************************************
		//1、验证玩家数据有效性
		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user != NULL)
		{
			LOG_MESSAGE("onCMD_%d err user != NULL...%d  line:%d \n", cmd, data.userindex, __LINE__);
			sendErrInfo(__TcpServer, c->ID, cmd, 3040, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//2、添加新玩家数据
		user = __UserManager->popUser();
		user->reset();
		bool isok = __UserManager->insertUser(data.userindex, user);
		if (isok == false)
		{
			LOG_MESSAGE("onCMD_%d err  isok=false...%d  line:%d \n", cmd, data.userindex, __LINE__);
			sendErrInfo(__TcpServer, c->ID, cmd, 3041, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//1、读取数据
		tc->read(&user->mem, sizeof(S_USER_MEMBER_DATA));
		tc->read(&user->role, sizeof(S_USER_ROLE));

		auto copy_map = script::findScript_Map(data.mapid);
		if (copy_map == nullptr)
		{
			LOG_MESSAGE("onCMD_%d err  copy_map=NULL...%d  line:%d \n", cmd, data.userindex, __LINE__);
			sendErrInfo(__TcpServer, c->ID, cmd, 3042, &data2, sizeof(S_CMD_COPY));
			return;
		}
		//查找自己的队伍 查找房间
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
		//切换地图成功
		user->changeMap(&data2, sizeof(S_CMD_COPY), c->ID, cmd, ECK_SUCCESS);
		user->role.base.status.mapid = data.mapid;
		user->initBornPos(data.mapid);
		user->node.layer = room->index;
		
		//通知组队服务器 玩家切换地图
		TS_Broadcast::team_ChangeMap(user, CMD_901);
	
		//通知网关 玩家切换地图
		TS_Broadcast::do_ChangeMap(user, &data2, sizeof(S_CMD_COPY), c->ID, cmd);


		LOG_MESSAGE("[db loadrole] 7301  mem:%d/%lld layer:%d mapid:%d/%d copy:%d team:%d/%d\n",
			user->node.index,user->mem.id, user->node.layer,data2.mapid,data2.user_connectid,
			user->tmp.temp_CopyIndex, user->tmp.temp_TeamIndex, user->tmp.temp_MateIndex);
	}

	//7301 收到组队服务器发来 开始进入副本的消息
	void onCMD_7301_Team(net::ITCPClient* tc)
	{
		S_CMD_COPY data;
		tc->read(&data, sizeof(S_CMD_COPY));

		//1、验证玩家
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
		//进入副本之前 先保存下当前所在地图ID和坐标 用于退出副本还原地图和坐标
		user->role.base.status.copyMapid(false);
		//4、获取需要去副本地图所在的服务器ID
		u32 serverid = share::findGameServerID(share::__ServerLine, data.mapid);
		//要去的服务器就是当前所在服务器 就无须切换
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
			//设置副本 开始刷怪
			user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_NO);
			user->role.base.status.mapid = data.mapid;
			user->initBornPos(data.mapid);
			user->node.layer = room->index;
			user->tmp.temp_CopyIndex = room->index;
			data.user_connectid = user->tmp.user_connectid;
			
			//通知组队服务器 玩家切换地图
		    TS_Broadcast::team_ChangeMap(user, CMD_901);
			//返回切换地图成功
			TS_Broadcast::do_ChangeMap(user, &data, sizeof(S_CMD_COPY), c->ID, CMD_7300);
			LOG_MESSAGE("[TeamServer] onCMD_7301 copy changeMap success...%d:%lld mapid:%d %d\n", data.userindex, data.memid, data.mapid, c->ID);
			return;
		}
		LOG_MESSAGE("[TeamServer] onCMD_7301 copy changeMap start.....index:%d %lld %d \n", data.userindex, data.memid, data.mapid);

		user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_YES);
		//通知DB服务器 开始切换地图  
		TS_Broadcast::db_ChangeMap(c, &data, data.userindex, share::__ServerLine, data.mapid, CMD_7301, 3000);
	}
	bool AppCopy::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_DB && tc->getData()->serverType != func::S_TYPE_CENTER) return false;

		if (tc->getData()->serverType == func::S_TYPE_CENTER)
		{
			switch (cmd)
			{
			case CMD_7301://进入副本
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