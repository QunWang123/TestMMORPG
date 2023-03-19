
#include  "AppBag.h"
#include  "APPGlobal.h"
#include  "UserManager.h"
#include  "WorldData.h"

#include  "CommandData.h"

namespace app
{
	IContainer*  __AppBag;


	AppBag::AppBag()
	{
	}

	AppBag::~AppBag()
	{
	}

	//820 获取其他玩家战斗装备
	void onGetCompat(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_COMBAT_DATA  data;
		ts->read(c->ID, data.userindex);

		S_USER_GATE* user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, 4001);
			return;
		}
		//控制下 获取战斗装备时间  3秒内 操作 返回操作过快错误
		//这个时间可以自己调整
		s32 ftime = time(NULL) - user->temp_GetCompatTime;
		if (ftime < USER_GETCOMPAT_TIME)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, 4002);
			return;
		}
		user->temp_GetCompatTime = time(NULL);

		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, 4003);
			return;
		}

		//发送给游戏服务器
		tcp->begin(CMD_820);
		tcp->sss(&data, sizeof(S_COMBAT_DATA));
		tcp->end();
	}


	//830 申请背包数据
	void onBag_Get(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_BAG_DATA  data;

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, 4001);
			return;
		}

		//控制下 获取背包时间  10秒内 操作 返回操作过快错误
		//这个时间可以自己调整
		s32 ftime = time(NULL) - user->temp_GetBagTime;
		if (ftime < USER_GETBAG_TIME)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, 4002);
			return;
		}
		user->temp_GetBagTime = time(NULL);

		//发生数据
		data.userindex = user->userindex;
		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, 4003);
			return;
		}

		//发送给游戏服务器
		tcp->begin(CMD_830);
		tcp->sss(&data, sizeof(S_BAG_DATA));
		tcp->end();

	}
	//840 整理背包数据 主要是合并道具数量
	void onBag_Combine(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_BAG_DATA  data;

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, 4001);
			return;
		}
		//控制下 获取背包时间  10秒内 操作 返回操作过快错误
		//这个时间可以自己调整
		s32 ftime = time(NULL) - user->temp_GetBagTime;
		if (ftime < USER_GETBAG_TIME)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, 4002);
			return;
		}
		user->temp_GetBagTime = time(NULL);

		data.userindex = user->userindex;
		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, 4003);
			return;
		}

		//发送给游戏服务器
		tcp->begin(CMD_830);
		tcp->sss(&data, sizeof(S_BAG_DATA));
		tcp->end();
	}
	//850 穿上装备
	//860 脱下装备
	//870 背包道具出售
	void onBag_EquipUpDown(net::ITCPServer* ts, net::S_CLIENT_BASE* c, u16 cmd)
	{
		S_EQUIPUPDOWN_DATA  data;
		ts->read(c->ID, data.pos);//背包或者战斗装备中的位置
		ts->read(c->ID, data.propid);
		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 4001);
			return;
		}
		data.userindex = user->userindex;
		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 4003);
			return;
		}

		//发送给游戏服务器
		tcp->begin(cmd);
		tcp->sss(&data, sizeof(S_EQUIPUPDOWN_DATA));
		tcp->end();
	}

	void onBag_EquipSwap(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_BAGEQUIPSWAP_DATA data;
		ts->read(c->ID, data.pos1);//背包或者战斗装备中的位置
		ts->read(c->ID, data.propid1);
		ts->read(c->ID, data.pos2);//背包或者战斗装备中的位置
		ts->read(c->ID, data.propid2);
		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_890, 4001);
			return;
		}
		data.userindex = user->userindex;
		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_890, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_890, 4003);
			return;
		}

		//发送给游戏服务器
		tcp->begin(CMD_890);
		tcp->sss(&data, sizeof(S_BAGEQUIPSWAP_DATA));
		tcp->end();
	}

	bool AppBag::onServerCommand(net::ITCPServer * ts, net::S_CLIENT_BASE * c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppLogin err...line:%d \n", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_USER) return false;
		if (c->state != func::S_LOGIN) return false;

		switch (cmd)
		{
			case CMD_820:onGetCompat(ts, c);			break;//获取其他玩家战斗装备数据
			case CMD_830:onBag_Get(ts, c);				break;//获取背包数据
			case CMD_840:onBag_Combine(ts, c);			break;//整理背包数据
			case CMD_850:onBag_EquipUpDown(ts, c, cmd); break;//穿上装备
			case CMD_860:onBag_EquipUpDown(ts, c, cmd); break;//脱下装备
			case CMD_870:onBag_EquipUpDown(ts, c, cmd); break;//道具出售
			case CMD_890:onBag_EquipSwap(ts, c);		break;
			break;
		}
		return true;
	}

	//***************************************************************************
	//***************************************************************************
	//***************************************************************************
	//更新背包道具
	void onCMD_800(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		u8 bagpos = 0;
		bool iscount = false;
		u16 count = 0;
		S_ROLE_PROP prop;
		prop.reset();
	
		tc->read(connectid);
		tc->read(memid);
		tc->read(bagpos);
		tc->read(iscount);
		if (iscount)
		{
			tc->read(count);
		}
		else
		{
			tc->read(&prop, prop.sendSize());
		}
	
		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", connectid, memid, __LINE__);
			return;
		}
	
		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_800);
		__TcpServer->sss(c->ID, bagpos);
		__TcpServer->sss(c->ID, iscount);
		if (iscount)
		{
			__TcpServer->sss(c->ID, count);
		}
		else
		{
			__TcpServer->sss(c->ID, &prop, prop.sendSize());
		}
		__TcpServer->end(c->ID);
	
	}
	//获取战斗装备
	void onCMD_820(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_COMBAT_DATA data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_COMBAT_DATA));
		
		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
	
		//1 验证是不是错误
		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, childcmd);
			return;
		}
	
		//只发有效数据
		u8 num = 0;
		tc->read(num);
		__TcpServer->begin(c->ID, CMD_820);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, data.userindex);
		__TcpServer->sss(c->ID, num);
		if (num > 0)
		{
			for (u8 i = 0; i < num; i++)
			{
				u8 bagpos = 0;
				app::S_ROLE_PROP prop;
				prop.reset();
				tc->read(bagpos);// 战斗装备中的位置
				tc->read(&prop, prop.sendSize());
				__TcpServer->sss(c->ID, bagpos);
				__TcpServer->sss(c->ID, &prop, prop.sendSize());
			}
		}
	
		__TcpServer->end(c->ID);
	}
	//获取背包全部道具
	void onCMD_830(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_BAG_DATA data;
		u8 bagnum = 0;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_BAG_DATA));
		tc->read(bagnum);
	
		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
	
		//1 验证是不是错误
		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, childcmd);
			return;
		}
	
		//2 解析 发送数据
		u8 num = 0;
		tc->read(num);
		__TcpServer->begin(c->ID, CMD_830);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, bagnum);
		__TcpServer->sss(c->ID, num);
		if (num > 0)
		{
			for (u8 i = 0; i < num; i++)
			{
				u8 bagpos = 0;
				app::S_ROLE_PROP prop;
				prop.reset();
				tc->read(bagpos);//背包中的位置
				tc->read(&prop, prop.sendSize());
	
				__TcpServer->sss(c->ID, bagpos);
				__TcpServer->sss(c->ID, &prop, prop.sendSize());
			}
		}
		__TcpServer->end(c->ID);
	}
	
	//获取背包全部道具(整理装备后发送的)
	void onCMD_840(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_BAG_DATA data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_DROP_DATA));
	
		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
	
		//1 验证是不是错误
		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, childcmd);
			return;
		}
	
		//2 解析 发送数据
		u8 num = 0;
		tc->read(num);
		__TcpServer->begin(c->ID, CMD_840);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, num);
		if (num > 0)
		{
			for (u8 i = 0; i < num; i++)
			{
				u8 bagpos = 0;
				u8 count = 0;
				tc->read(bagpos);//背包中的位置
				tc->read(count); //当前数量
	
				__TcpServer->sss(c->ID, bagpos);
				__TcpServer->sss(c->ID, count);
			}
		}
		__TcpServer->end(c->ID);
	}
	//810 穿上或者脱下装备
	void onCMD_810(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		u8 kind = 0;
		u8 bagpos = 0;
		u8 equippos = 0;
	
		tc->read(connectid);
		tc->read(memid);
		tc->read(kind);
		tc->read(bagpos);
		tc->read(equippos);
	
		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", connectid, memid, __LINE__);
			return;
		}
	
		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_810);
		__TcpServer->sss(c->ID, kind);
		__TcpServer->sss(c->ID, bagpos);
		__TcpServer->sss(c->ID, equippos);
		__TcpServer->end(c->ID);
	
	}
	//穿上装备 错误数据 返回,正确的穿上装备是CMD_800
	void onCMD_850(net::ITCPClient* tc,u16 cmd)
	{
		u16 childcmd = 0;
		S_EQUIPUPDOWN_DATA data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_DROP_DATA));
	
		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
	
		//1 验证是不是错误
		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, childcmd);
			return;
		}
	}
	bool AppBag::onClientCommand(net::ITCPClient * tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_GAME) return false;
	
		switch (cmd)
		{
		case CMD_800:  onCMD_800(tc);  break;//更新背包中的道具
		case CMD_820:  onCMD_820(tc);  break;//获取战斗装备
		case CMD_830:  onCMD_830(tc);  break;//获取背包数据
		case CMD_840:  onCMD_840(tc);  break;//整理背包数据
		case CMD_810:  onCMD_810(tc);  break;//穿上或者脱下 更新数据
		case CMD_850: 
		case CMD_860:
		case CMD_870:
			onCMD_850(tc,cmd);  break;//穿上或者脱下错误返回 出售金币
		}
		return true;
	}
}