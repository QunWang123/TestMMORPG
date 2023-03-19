
#include  "AppDrop.h"
#include  "WorldMap.h"
#include  "UserManager.h"
#include  "WorldTools.h"
#include  "WorldBC.h"
#include "AppGlobal.h"
namespace app
{
	IContainer*  __AppDrop;
	u32  checkmapid = 0;
	s64  checktime = 0;

	AppDrop::AppDrop(){}
	AppDrop::~AppDrop(){}

	void AppDrop::onUpdate()
	{
		//200毫秒检查一次  一次检查一张地图
		s32 value = global_gametime - checktime;
		if (value < 200) return;
		checktime = global_gametime;
        
		if (checkmapid >= MAX_MAP_ID) checkmapid = 0;
		S_WORLD_MAP* map = nullptr;
		bool isfind = false;
		for (u32 i = checkmapid; i < MAX_MAP_ID; i++)
		{
			map = __World->getMap(i);
			if (map == nullptr) continue;
			if (!map->isUsed)   continue;
			checkmapid = i + 1;
			isfind = true;
			break;
		}
		if (!isfind)
		{
			for (u32 i = 0; i < MAX_MAP_ID; i++)
			{
				map = __World->getMap(i);
				if (map == nullptr) continue;
				if (!map->isUsed)   continue;
				checkmapid = i + 1;
				isfind = true;
				break;
			}
		}
		if (!isfind) return;
		

		S_RECT_BASE* edge = &map->Node.rect;
		for (u32 row = edge->top; row <= edge->bottom; row++)
		for (u32 col = edge->left; col <= edge->right; col++)
		{
			if (!map->IsRect(row, col)) continue;

			S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
			while (node != nullptr)
			{
				if (node->type != N_PROP)
				{
					node = node->downnode;
					continue;
				}
				S_WORLD_DROP_BASE* drop = map->Drop.findDrop(node->index);
				if (drop == nullptr)
				{
					node = node->downnode;
					continue;
				}
				s32 value = global_gametime - drop->lock_time;
				u32 index = node->index;
				s32 layer = node->layer;
				node = node->downnode;
			
				//超过保护时间
				if (value > 100000 && value < 120000)
				{
					if (drop->lock_index >= 0 || drop->lock_teamid > 0)
					{
						drop->lock_index = -1;
						drop->lock_teamid = -1;

						LOG_MESSAGE("AppDrop 30秒时间到 掉落设置为无主:mapid:%d/%d big:%d/%d\n",
							map->mapID,index, drop->bc.grid_big.row, drop->bc.grid_big.col);
					}
				}
				else if (value >= 120000)
				{
					LOG_MESSAGE("AppDrop 120秒时间到 掉落删除:mapid:%d/%d big:%d/%d\n",
						map->mapID, index, drop->bc.grid_big.row, drop->bc.grid_big.col);

					TS_Broadcast::bc_DropDelete(map->mapID, index);
					map->Drop.leaveWorld(map->mapID, index, layer);
				}
			}
		}
	}

	void AppDrop::clearDrop(s32 mapid,s32 layer)
	{
		auto map = __World->getMap(mapid);
		if (map == NULL) return;


		S_RECT_BASE* edge = &map->Node.rect;

		LOG_MESSAGE("clearDrop....mapid:%d layer:%d count:%d edge:%d/%d  %d/%d \n",mapid,layer,map->Drop.curcount,
			edge->top, edge->bottom, edge->left, edge->right);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		for (u32 col = edge->left; col <= edge->right; col++)
		{
			if (!map->IsRect(row, col))  continue;

			S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
			while (node != nullptr)
			{
				if (node->type != N_PROP)
				{
					node = node->downnode;
					continue;
				}
				S_WORLD_DROP_BASE* drop = map->Drop.findDrop(node->index);
				if (drop == nullptr)
				{
					node = node->downnode;
					continue;
				}
				if(drop->node.layer != layer)
				{
					node = node->downnode;
					continue;
				}

				s32 index = node->index;
				node = node->downnode;

				map->Drop.leaveWorld(map->mapID, index, layer);
			}
		}
	}
	bool AppDrop::onServerCommand(net::ITCPServer * ts, net::S_CLIENT_BASE * c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppSkill close...line:%d \n", __LINE__);
			return false;
		}
		if (c->clientType != func::S_TYPE_GATE)
		{
			LOG_MESSAGE("AppDrop close...line:%d \n", __LINE__);
			return false;
		}

		switch (cmd)
		{
		case CMD_6000:
			OnGet(ts, c);
			break;
		}
				
		return true;
	}

	void sendErrInfo(s32 connectid, u16 cmd, u16 childcmd, S_DROP_DATA* data)
	{
		__TcpServer->begin(connectid, cmd);
		__TcpServer->sss(connectid, childcmd);
		__TcpServer->sss(connectid, data, sizeof(S_DROP_DATA));
		__TcpServer->end(connectid);
	}


	//11000-2000 拾取某个掉落物品
	void AppDrop::OnGet(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_DROP_DATA data;
		ts->read(c->ID, &data, sizeof(S_DROP_DATA));


		//1、查找使用技能玩家
		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_6000, 3001, &data);
			return;
		}

		//2、验证地图
		S_WORLD_MAP*  map = __World->getMap(user->role.base.status.mapid);
		if (map == nullptr)
		{
			sendErrInfo(c->ID, CMD_6000, 3002, &data);
			return;
		}

		//3 上限
		if (data.dropindex >= map->Drop.maxcount)
		{
			sendErrInfo(c->ID, CMD_6000, 3003, &data);
			return;
		}

		//2、验证掉落
		S_WORLD_DROP_BASE* drop = map->Drop.findDrop_Safe(data.dropindex, user->node.layer);
		if(drop == NULL || !drop->isused)
		{
			sendErrInfo(c->ID, CMD_6000, 3004, &data);
			return;
		}
		
		if (drop->prop.base.id != data.dropid)
		{
			sendErrInfo(c->ID, CMD_6000, 3005, &data);
			return;
		}
		//5、距离验证 超过4米
		u32 distance = TS_Tools::getDistance(&user->bc.grid_pos, &drop->bc.grid_pos);
		if (distance > 3)
		{
			sendErrInfo(c->ID, CMD_6000, 3006, &data);
			return;
		}
		// 有锁定的拾取玩家
		if (drop->lock_index >= 0 && drop->lock_index != user->node.index)
		{
			sendErrInfo(c->ID, CMD_6000, 3007, &data);
			return;
		}

		switch (drop->prop.base.type)
		{
		case EPT_GOLD:
			user->role.base.econ.gold += drop->prop.base.money;
			TS_Broadcast::do_SendGold(user);//更新经济
			TS_Broadcast::db_SaveGold(user);
			break;
		case EPT_EQUIP:
		case EPT_CONSUME:
			{
			    //无效的道具 查看表是不是有这个ID
 			    bool isvalid = drop->prop.isPropValid();
 				if (isvalid == false)
 				{
 					sendErrInfo(c->ID, CMD_6000, 3008, &data);
 					return;
 				}

				//往背包添加道具
				script::SCRIPT_PROP* s = (script::SCRIPT_PROP*)drop->prop.temp.script_prop;
				s32 maxcount = s->maxcount;
				if (maxcount <= 0)
				{
					sendErrInfo(c->ID, CMD_6000, 3009, &data);
					return;
				}
				//验证背包满了吗？
				bool iscount = false;
				s32 bagpos = user->bag_AddProp(&drop->prop, maxcount, iscount);
				if(bagpos == -1)
				{
					sendErrInfo(c->ID, CMD_6000, 3010, &data);
					return;
				}
				//回传DB保存
				TS_Broadcast::db_SendUpdateProp(user,bagpos,iscount);
				//发送背包道具更新
				TS_Broadcast::do_SendUpdateProp(user,bagpos,iscount);
			}
			break;
		}
		LOG_MESSAGE("你拾取了道具:%d \n", data.dropid);
		
		//广播删除道具 离开世界
		TS_Broadcast::bc_DropDelete(user->role.base.status.mapid, data.dropindex);
		map->Drop.leaveWorld(map->mapID, drop->node.index, drop->node.layer);
			
		// 设置任务
		// user->mainTask_Update(0);
	}
}