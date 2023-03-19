#include "AppEnterWorld.h"
#include "CommandData.h"
#include "WorldData.h"
#include "UserManager.h"
#include "AppGlobal.h"
#include "WorldBC.h"
#include "ShareFunction.h"
namespace app
{
	AppEnterWorld* __AppEntry = nullptr;

	AppEnterWorld::AppEnterWorld()
	{

	}

	AppEnterWorld::~AppEnterWorld()
	{

	}

	void AppEnterWorld::onInit()
	{

	}

	// CMD_700
	void onGetOtherRoleData(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_GETROLE_DATA data;
		ts->read(c->ID, &data, sizeof(S_GETROLE_DATA));

		auto targetuser = __UserManager->findUser(data.targetindex);
		if (targetuser == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_700, 3001, &data, sizeof(S_GETROLE_DATA));
			return;
		}

		__TcpServer->begin(c->ID, CMD_700);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, &data, sizeof(S_GETROLE_DATA));
		__TcpServer->sss(c->ID, &targetuser->role.base.innate, C_ROLE_INNATE_LEN);
		__TcpServer->sss(c->ID, &targetuser->role.base.status, C_ROLE_STATUS_LEN);
		__TcpServer->sss(c->ID, &targetuser->role.base.exp, C_ROLE_EXP_LEN);
		__TcpServer->sss(c->ID, &targetuser->role.base.life, sizeof(S_ROLE_BASE_LIFE));
		__TcpServer->end(c->ID);
		
		// 获取别的玩家时，顺便把buff也给返回去
		for (u32 i = 0; i < MAX_BUFF_COUNT; i++)
		{
			S_TEMP_BUFF_RUN_BASE* run = &targetuser->tmp.temp_BuffRun.data[i];
			if (run->buff_id <= 0) continue;
			if (run->runningtime <= 200) continue;
			TS_Broadcast::do_SendBuff(CMD_5000, c->ID, data.user_connectid, data.memid, targetuser->node.index, run);
		}
	}

	void onEntryWorld(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_ENTRY_BASE data;
		ts->read(c->ID, &data, sizeof(S_ENTRY_BASE));

		// 在selectrole中的onCMD_500中插入的
		auto user = __UserManager->findUser(data.userindex, data.memid);
		if (user == NULL)
		{
			sendErrInfo(ts, c->ID, CMD_900, 3001, &data, sizeof(S_GETROLE_DATA));
			return;
		}

		user->enterWorld();
	}


	//901 切换地图
	//902 换线
	void onChangeMap(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		u16 childcmd = 0;
		S_CHANGEMAP_BASE data;
		ts->read(c->ID, childcmd);
		ts->read(c->ID, &data, sizeof(S_CHANGEMAP_BASE));

		// 切线路和部分情况下的切地图（会切gameserver的情况下）
		//							DBServer											DBServer
		//			②↑childcmd=3000	 ③↓childcmd=3000(原样返回)		⑥↑childcmd=0		
		//			原gameserver			原gameserver				玩家的新gameserver
		// ①↑childcmd=0						④↓childcmd=3000			⑤↑childcmd=1	
		// gateserver						gateserver(更新玩家连的gameserver)
		//  ↑902（901）
		// 玩家

		// 如果不用换gameserver，那连第二部都不用走

		//1、验证玩家
		S_USER_BASE* user = nullptr;
		if (childcmd == 1) //加载角色角色数据
		{
			// 
			//1、验证玩家数据有效性 该地图已经有玩家数据 错误
			user = __UserManager->findUser(data.userindex);
			if (user != nullptr)
			{
				sendErrInfo(ts, c->ID, cmd, 3001, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
		}
		else
		{
			//1、验证玩家数据有效性 必须是唯一索引+账号ID
			user = __UserManager->findUser(data.userindex, data.memid);
			if (user == nullptr)
			{
				sendErrInfo(ts, c->ID, cmd, 3001, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
		}

		//2、验证DB连接
		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			sendErrInfo(ts, c->ID, cmd, 3002, &data, sizeof(S_CHANGEMAP_BASE));
			return;
		}

		//申请加载角色数据
		if (childcmd == 1)
		{
			__UserManager->onPushLoadRole(c, &data, data.userindex, data.mapid, cmd, childcmd);
			return;
		}

		//切换地图
		if (cmd == CMD_901)
		{
			//3、验证地图ID有效性
			if (data.mapid >= MAX_MAP_ID)
			{
				sendErrInfo(ts, c->ID, cmd, 3003, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}

			//4、要去的地图ID和现在的是一样的
			if (data.mapid == user->role.base.status.mapid)
			{
				sendErrInfo(ts, c->ID, cmd, 3004, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
		}
		else //切换线路
		{
			//3、要去的线路和现在的是一样的
			if (data.line == share::__ServerLine)
			{
				sendErrInfo(ts, c->ID, cmd, 3003, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
			//4、验证地图ID有效性
			if (data.mapid != user->role.base.status.mapid)
			{
				sendErrInfo(ts, c->ID, cmd, 3004, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}
		}


		//获取需要切换地图所在的服务器ID
		//要去的服务器就是当前所在服务器 就无须切换
		//换线的话 必须是其他进程
		u32 serverid = share::findGameServerID(share::__ServerLine, data.mapid);
		if (serverid == func::__ServerInfo->ID)
		{
			// 进来这里说明是在同一个gameserver下切图，只需更新部分数据即可
			// 切换线路肯定不能在同一个进程
			if (cmd == CMD_902)
			{
				sendErrInfo(ts, c->ID, cmd, 3005, &data, sizeof(S_CHANGEMAP_BASE));
				return;
			}

			user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_NO);

			user->role.base.status.mapid = data.mapid;
			user->initBornPos(data.mapid);
			user->role.base.status.pos.z = 0;
			
			//通知组队服务器 玩家切换地图
			TS_Broadcast::team_ChangeMap(user, CMD_901);
			//返回切换地图成功
			TS_Broadcast::do_ChangeMap(user, &data, sizeof(S_CHANGEMAP_BASE), c->ID, cmd);

			LOG_MESSAGE("changeMap success...%d \n", data.mapid);
			return;
		}
		// 走到这说明两种情况：
		// ①同一地图要切线
		// ②同一线要切图导致的切换gameserver

		LOG_MESSAGE("changeMap start.....index:%d %lld %d \n", data.userindex, data.memid, data.mapid);

		// 通知DB服务器 开始切换地图  这里是个巨坑 一定要注意:
		// 这里一定要通知DB 必须等DB返回成功消息 才能进行切换  
		// 因为要确保玩家的数据都同步到了DB 才能进行下一步操作
		// 如：背包操作后，然后开始选择换线，这个时候如果背包数据还没同步到DB 
		// 数据还在缓存中 有可能DB服务器IO处理繁忙，这个时候还在缓存队列
		// 这个时候就直接换线 切换到其他进程后 去LOAD数据 
		// 有可能加载出来的是脏数据 所以必须要确保DB数据同步完毕了来。。。
		// 因为TCP都是有序的 所以发送下面这个消息 就说明了全部同步完毕了
		user->changeMap(nullptr, 0, 0, 0, ECK_LEAVE_YES);

		TS_Broadcast::db_ChangeMap(c, &data, data.userindex, data.line, data.mapid, cmd, 3000);
	}

	bool AppEnterWorld::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppEnterWorld err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_700:
			onGetOtherRoleData(ts, c);
			break;
		case CMD_900:
			onEntryWorld(ts, c);
			break;
		case CMD_901: //切换地图 和 换线 一个逻辑
		case CMD_902:
			onChangeMap(ts, c, cmd); 
			break; //切换线路
		}
		return true;
	}

	//****************************************************************************
	//****************************************************************************
	//901 902 收到DB服务器返回的加载角色数据 或者DB数据同步完毕
	void onCMD_901(net::ITCPClient* tc, const u16 cmd)
	{
		u16 childcmd = 0;
		S_LOAD_ROLE data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_LOAD_ROLE));

		S_CHANGEMAP_BASE data2;
		data2.userindex = data.userindex;
		data2.memid = data.memid;
		data2.user_connectid = data.user_connectid;
		data2.mapid = data.mapid;
		data2.line == data.line;

		auto c = __TcpServer->client(data.server_connectid, data.server_clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("onCMD_%d c == NULL...%d %d line:%d \n", cmd, data.server_connectid, data.server_clientid, __LINE__);
			return;
		}
		if (childcmd == 3000)
		{
			//返回给网关服务器 需要切换到另外个进程
			sendErrInfo(__TcpServer, c->ID, cmd, 3000, &data2, sizeof(S_CHANGEMAP_BASE));
			return;
		}
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, childcmd, &data2, sizeof(S_CHANGEMAP_BASE));
			return;
		}

		//1、验证玩家数据有效性
		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user != NULL)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3020, &data2, sizeof(S_CHANGEMAP_BASE));
			return;
		}
		//2、添加新玩家数据
		user = __UserManager->popUser();
		user->reset();
		bool isok = __UserManager->insertUser(data.userindex, user);
		if (isok == false)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 3021, &data2, sizeof(S_CHANGEMAP_BASE));
			return;
		}

		//1、读取数据
		tc->read(&user->mem, sizeof(S_USER_MEMBER_DATA));
		tc->read(&user->role, sizeof(S_USER_ROLE));

		//2、只有切换地图才会生成新坐标 换线不生成新坐标 直接使用原来坐标
		if (cmd == CMD_901)
		{
			//2、切换地图 需要生成新坐标
			s32 kind = user->initBornPos(data.mapid);
			if (kind != 0)
			{
				sendErrInfo(__TcpServer, c->ID, cmd, 3022, &data2, sizeof(S_CHANGEMAP_BASE));
				return;
			}
			user->role.base.status.mapid = data.mapid;
		}


		user->tmp.userindex = data.userindex;
		user->tmp.server_connectid = c->ID;
		user->tmp.server_clientid = c->clientID;
		user->tmp.user_connectid = data.user_connectid;
		user->tmp.temp_HeartTime = (int)time(NULL);
		
		//切换地图成功
		user->changeMap(&data2, sizeof(S_CHANGEMAP_BASE), c->ID, cmd, ECK_SUCCESS);
		//通知组队服务器 玩家切换地图
		TS_Broadcast::team_ChangeMap(user, CMD_901);
		//返回切换地图成功
		TS_Broadcast::do_ChangeMap(user, &data2, sizeof(S_CHANGEMAP_BASE), c->ID, cmd);

		LOG_MESSAGE("changemap success........................\n");

	}

	bool AppEnterWorld::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_DB) return false;


		switch (cmd)
		{
		case CMD_901:
		case CMD_902:
			onCMD_901(tc, cmd);
			break;
		}

		return false;
	}
}



