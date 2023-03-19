#include "WorldBC.h"
#include "WorldMap.h"
#include "AppGlobal.h"
#include "ShareFunction.h"
#include "AppTeam.h"
#include "UserManager.h"


namespace app
{
	void TS_Broadcast::team_StartCopy(S_CMD_COPY& data, const u16 cmd)
	{
		if (__TcpCenter == nullptr) return;
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE) return;

		__TcpCenter->begin(cmd);
		__TcpCenter->sss(&data, sizeof(S_CMD_COPY));
		__TcpCenter->end();
	}

	//7400 离开副本
	void TS_Broadcast::team_LeaveCopy(S_USER_BASE* user, s32 copyindex)
	{
		if (__TcpCenter == nullptr) return;
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE) return;

		S_CMD_COPY data;
		data.userindex = user->tmp.userindex;
		data.memid = user->mem.id;
		data.roomindex = copyindex;
		data.teamindex = user->tmp.temp_TeamIndex;
		data.mapid = user->role.base.status.mapid;

		__TcpCenter->begin(CMD_7400);
		__TcpCenter->sss(&data, sizeof(S_CMD_COPY));
		__TcpCenter->end();
	}

	void TS_Broadcast::bc_ResetCopy(S_COPY_ROOM* room, u16 childcmd)
	{
		if (room == nullptr) return;
		auto team = __AppTeam->findTeam(room->teamindex);
		if (team == nullptr) return;

		for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
		{
			auto mate = team->findMate(i);
			if (mate == nullptr) continue;

			auto user = __UserManager->findUser(mate->userindex, mate->memid);
			if (user == nullptr) continue;
			if (user->tmp.temp_CopyIndex != room->index) continue;

			auto c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
			if (c == nullptr) continue;
			S_CMD_COPY data;
			data.userindex = user->tmp.userindex;
			data.memid = user->mem.id;
			data.user_connectid = user->tmp.user_connectid;
			data.roomindex = room->index;
			data.teamindex = team->index;

			__TcpServer->begin(c->ID, CMD_7401);
			__TcpServer->sss(c->ID, childcmd);
			__TcpServer->sss(c->ID, &data, sizeof(S_CMD_COPY));
			__TcpServer->end(c->ID);
		}
	}

	// 九宫格范围内广播玩家离开世界
	void TS_Broadcast::bc_RoleLeaveWorld(S_USER_BASE* user)
	{
		if (user->role.base.status.state == GAME_FREE) return;
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);

		if (map == NULL) return;
		S_RECT_BASE* edge = &user->bc.edge;
		map->IsRect(edge);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (map->IsRect(row, col))
				{
					// 广播
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}
						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, false);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						LOG_MESSAGE("BC UserLeaveWorld %lld-%d\n", user->mem.id, user->node.index);
						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_600);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->tmp.user_connectid);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->mem.id);
						__TcpServer->sss(otheruser->tmp.server_connectid, user->node.index);
						__TcpServer->end(otheruser->tmp.server_connectid);
						 
						node = node->downnode;
					}
				}
			}
		}
	}
	// 服务器3-5秒推送到九宫格玩家，新玩家进入游戏时，就靠这个获取其他玩家的数据
	void TS_Broadcast::bc_RoleState(S_USER_BASE* user)
	{
		s32 tempTime = (s32)time(NULL) - user->tmp.temp_BCTime;
		if (tempTime < 3) return;
		user->tmp.temp_BCTime = (s32)time(NULL);

		S_ROLE_BASE_STATUS* status = &user->role.base.status;
		bool isup = true;
		
		if (user->role.base.life.hp == 0 || user->role.base.life.hp >= 100)
		{
			isup = false;
		}
		// 获取世界地图
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return;
		// 获取玩家广播的区域
		S_RECT_BASE* edge = &user->bc.edge;
		map->IsRect(edge);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (map->IsRect(row, col))
				{
					// 广播
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						// 角色类型不是N_ROLE就不广播
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}
						// node中记录的不能是自己，从node转为玩家
						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, false);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						if (isup)
						{
							do_SendValue(CMD_710, user->node.index, user->role.base.life.hp, otheruser);
						}
						else
						{
							do_SendValue(CMD_730, user->node.index, user->role.base.status.state, otheruser);
						}
						node = node->downnode;
					}
				}
			}
		}
	}
	// 1100 广播xx玩家在世界移动
	void TS_Broadcast::bc_RoleMove(S_USER_BASE* user, S_MOVE_ROLE* move)
	{
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return;

		S_RECT_BASE* edge = &user->bc.edge;
		map->IsRect(edge);

		// 遍历九宫格范围
		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (map->IsRect(row, col))
				{
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}

						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, false);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}

						move->user_connectid = otheruser->tmp.user_connectid;
						move->memid = otheruser->mem.id;

						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_1100);
						__TcpServer->sss(otheruser->tmp.server_connectid, move, sizeof(S_MOVE_ROLE));
						__TcpServer->end(otheruser->tmp.server_connectid);

						node = node->downnode;
					}
				}
			}
		}
	}
	// xx玩家离开大格子
	void TS_Broadcast::bc_RoleMoveLeaveBig(S_RECT_BASE* old, S_USER_BASE* user)
	{
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return;

		map->IsRect(old);

		for (u32 row = old->top; row <= old->bottom; row++)
		{
			for (u32 col = old->left; col <= old->right; col++)
			{
				if (map->IsRect(row, col))
				{
					S_GRID_BASE grid;
					grid.col = col;
					grid.row = row;

					// 这个地方的edge已经更换为新的,遍历新的九宫格，如果某个grid在旧的edge里面旧continue
					if (user->bc.edge.inEdge(&grid)) continue;
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}

						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, false);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						LOG_MESSAGE("BC user move leave...%lld-%d", user->mem.id, user->node.index);

						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_600);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->tmp.user_connectid);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->mem.id);
						__TcpServer->sss(otheruser->tmp.server_connectid, user->node.index);
						__TcpServer->end(otheruser->tmp.server_connectid);

						node = node->downnode;
					}
				}
			}
		}
	}


	//广播玩家升级
	void TS_Broadcast::bc_RoleLevelUP(S_USER_BASE* user)
	{
		if (user->role.base.status.state == GAME_FREE) return;
		auto map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return;

		//获取九宫格区域
		S_RECT_BASE * edge = &user->bc.edge;
		map->IsRect(edge);

		//遍历九宫格
		for (u32 row = edge->top; row <= edge->bottom; row++)
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (map->IsRect(row, col) == false) continue;

				//遍历大格子容器
				S_WORLD_NODE * node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					if (node->type != N_ROLE)
					{
						node = node->downnode;
						continue;
					}
					S_USER_BASE* other = node->nodeToUser(user->node.layer, user->node.index, false);
					if (other == nullptr || other->role.base.status.mapid != user->role.base.status.mapid)
					{
						node = node->downnode;
						continue;
					}
					LOG_MESSAGE("bc level...%lld-%d\n", user->mem.id, user->node.index);

					__TcpServer->begin(other->tmp.server_connectid, CMD_750);
					__TcpServer->sss(other->tmp.server_connectid, other->tmp.user_connectid);
					__TcpServer->sss(other->tmp.server_connectid, other->mem.id);
					__TcpServer->sss(other->tmp.server_connectid, user->node.index);
					__TcpServer->sss(other->tmp.server_connectid, user->role.base.exp.level);
					__TcpServer->end(other->tmp.server_connectid);

					node = node->downnode;
				}

			}
	}



	void TS_Broadcast::do_SendAtk(S_USER_BASE* user_connect, S_USER_BASE* user)
	{
		__TcpServer->begin(user_connect->tmp.server_connectid, CMD_770);
		__TcpServer->sss(user_connect->tmp.server_connectid, user_connect->tmp.user_connectid);
		__TcpServer->sss(user_connect->tmp.server_connectid, user_connect->mem.id);
		__TcpServer->sss(user_connect->tmp.server_connectid, user->node.index);
		__TcpServer->sss(user_connect->tmp.server_connectid, &user->atk.total, sizeof(S_SPRITE_ATTACK));
		__TcpServer->end(user_connect->tmp.server_connectid);
	}

	// 发送数据，使user收到某位玩家的状态或生命数据	
	void TS_Broadcast::do_SendValue(u16 cmd, u32 index, s32 value, S_USER_BASE* user)
	{
		// v自己是发送给其他玩家的，
		S_UPDATE_VALUE v;
		// value即发送给其他玩家的自己的数据
		v.value = value;
		// targetindex发送给其他玩家的自己的信息
		v.targetindex = index;
		// memid是在网关时验证发送对象的，自然要是其他玩家的i=memid
		v.memid = user->mem.id;
		// user_connectid是其他玩家在网关上的id
		v.user_connectid = user->tmp.user_connectid;
		// user->tmp.server_connectid，当然要发送给其他玩家所在的网关服务器
		__TcpServer->begin(user->tmp.server_connectid, cmd);
		__TcpServer->sss(user->tmp.server_connectid, &v, sizeof(S_UPDATE_VALUE));
		__TcpServer->end(user->tmp.server_connectid);
	}

	void TS_Broadcast::do_SendExp(S_USER_BASE* user)
	{
		__TcpServer->begin(user->tmp.server_connectid, CMD_740);
		__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
		__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
		__TcpServer->sss(user->tmp.server_connectid, user->role.base.exp.currexp);
		__TcpServer->end(user->tmp.server_connectid);
	}

	void TS_Broadcast::do_SendGold(S_USER_BASE* user)
	{
		__TcpServer->begin(user->tmp.server_connectid, CMD_760);
		__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
		__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
		__TcpServer->sss(user->tmp.server_connectid, user->role.base.econ.gold);
		__TcpServer->sss(user->tmp.server_connectid, user->role.base.econ.diamonds);
		__TcpServer->end(user->tmp.server_connectid);
	}

	void TS_Broadcast::do_SendUpdateProp(S_USER_BASE* user, u8 bagpos, bool iscount)
	{
		if (bagpos >= USER_MAX_BAG) return;
		S_ROLE_PROP* prop = &user->role.stand.bag.bags[bagpos];

		__TcpServer->begin(user->tmp.server_connectid, CMD_800);
		__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
		__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
		__TcpServer->sss(user->tmp.server_connectid, bagpos); //背包位置
		__TcpServer->sss(user->tmp.server_connectid, iscount);//更新数量还是全部数据？
		//只更新数量
		if (iscount)
		{
			__TcpServer->sss(user->tmp.server_connectid, prop->base.count); //0 代表删除
		}
		else
		{
			__TcpServer->sss(user->tmp.server_connectid, prop, prop->sendSize());
		}
		__TcpServer->end(user->tmp.server_connectid);
	}

	void TS_Broadcast::do_SendCompat(S_USER_BASE* user, u8 bagpos, u8 equippos, u8 kind)
	{
		auto c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
		if (c == nullptr) return;

		__TcpServer->begin(c->ID, CMD_810);
		__TcpServer->sss(c->ID, user->tmp.user_connectid);
		__TcpServer->sss(c->ID, user->mem.id);
		__TcpServer->sss(c->ID, kind);
		__TcpServer->sss(c->ID, bagpos);
		__TcpServer->sss(c->ID, equippos);
		__TcpServer->end(c->ID);
	}

	void TS_Broadcast::do_SendSwap(S_USER_BASE* user, u8 equippos1, u8 equippos2)
	{
		auto c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
		if (c == nullptr) return;
		__TcpServer->begin(c->ID, CMD_890);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, user->tmp.user_connectid);
		__TcpServer->sss(c->ID, user->mem.id);
		__TcpServer->sss(c->ID, equippos1);
		__TcpServer->sss(c->ID, equippos2);
		__TcpServer->end(c->ID);
	}

	//发送切换地图成功消息
	void TS_Broadcast::do_ChangeMap(S_USER_BASE* user, void* d, u32 size, s32 connectid, u16 cmd)
	{
		//返回切换地图成功
		__TcpServer->begin(connectid, cmd);
		__TcpServer->sss(connectid, (u16)0);
		__TcpServer->sss(connectid, d, size);
		__TcpServer->sss(connectid, &user->role.base.status.pos, 12);
		__TcpServer->end(connectid);
	}
	//901 通知组队服务器 玩家切换地图
	void TS_Broadcast::team_ChangeMap(S_USER_BASE* user, const u16 cmd)
	{
		if (__TcpCenter == nullptr) return;
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE) return;

		S_CHANGEMAP_BASE data;
		data.userindex = user->tmp.userindex;
		data.memid = user->mem.id;
		data.user_connectid = user->tmp.user_connectid;
		data.mapid = user->role.base.status.mapid;
		data.line == share::__ServerLine;

		__TcpCenter->begin(cmd);
		__TcpCenter->sss(&data, sizeof(S_CHANGEMAP_BASE));
		__TcpCenter->end();
	}
}