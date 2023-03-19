#include "WorldBC.h"
#include "WorldMap.h"
#include "AppGlobal.h"
namespace app
{
	//发送掉落物品6100
	void TS_Broadcast::do_SendDropAdd(S_USER_BASE* user, void* _drop)
	{
		S_WORLD_DROP_BASE* drop = (S_WORLD_DROP_BASE*)_drop;
		if (drop == nullptr) return;

		s32 ftime = global_gametime - drop->lock_time;

		__TcpServer->begin(user->tmp.server_connectid, CMD_6100);
		__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
		__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
		__TcpServer->sss(user->tmp.server_connectid, ftime);
		__TcpServer->sss(user->tmp.server_connectid, drop->lock_index);
		__TcpServer->sss(user->tmp.server_connectid, drop->lock_teamid);
		__TcpServer->sss(user->tmp.server_connectid, drop->node.index);
		__TcpServer->sss(user->tmp.server_connectid, drop->prop.base.id);
		__TcpServer->sss(user->tmp.server_connectid, &drop->bc.grid_pos, sizeof(S_GRID_BASE));
		__TcpServer->end(user->tmp.server_connectid);

	}
	//发送掉落物品6200
	void TS_Broadcast::do_SendDropDelete(S_USER_BASE* user, void* _drop)
	{
		S_WORLD_DROP_BASE* drop = (S_WORLD_DROP_BASE*)_drop;
		if (drop == nullptr) return;

		__TcpServer->begin(user->tmp.server_connectid, CMD_6200);
		__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
		__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
		__TcpServer->sss(user->tmp.server_connectid, drop->node.index);
		__TcpServer->end(user->tmp.server_connectid);

	}
	
	//广播掉落信息
	void TS_Broadcast::bc_DropInfo(u32 mapid, u32 dropindex)
	{
		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return;
		if (dropindex >= map->Drop.maxcount) return;

		S_WORLD_DROP_BASE* drop = map->Drop.findDrop(dropindex);

		LOG_MESSAGE("item%d has droped, the dropindex is %d\n", drop->prop.base.id, dropindex);

		S_RECT_BASE* edge = &drop->bc.edge;
		map->IsRect(edge);

		for (u32 row = edge->top; row <= edge->bottom; row++)
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (!map->IsRect(row, col)) continue;

				S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					if (node->type != N_ROLE)
					{
						node = node->downnode;
						continue;
					}

					S_USER_BASE* other = node->nodeToUser(node->layer, node->index, true);
					if (other == nullptr || other->role.base.status.mapid != mapid)
					{
						node = node->downnode;
						continue;
					}
					do_SendDropAdd(other, drop);
					node = node->downnode;
				}
			}
	}

	//广播道具删除
	void TS_Broadcast::bc_DropDelete(u32 mapid, u32 dropindex)
	{
		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return;
		if (dropindex >= map->Drop.maxcount) return;

		S_WORLD_DROP_BASE* drop = map->Drop.findDrop(dropindex);

		// 以这个掉落物品的大区域进行广播
		S_RECT_BASE* edge = &drop->bc.edge;
		map->IsRect(edge);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (!map->IsRect(row, col)) continue;

				S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					if (node->type != N_ROLE)
					{
						node = node->downnode;
						continue;
					}
					S_USER_BASE* other = node->nodeToUser(node->layer, node->index, true);
					if (other == nullptr || other->role.base.status.mapid != mapid)
					{
						node = node->downnode;
						continue;
					}
					do_SendDropDelete(other, drop);
					node = node->downnode;
				}
			}
		}
	}
}