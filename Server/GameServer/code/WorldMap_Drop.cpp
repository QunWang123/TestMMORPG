#include "WorldMap.h"
#include <io.h>
#include <time.h>
#include "WorldMap.h"
#include "WorldTools.h"
#include "AppGlobal.h"
#include "WorldBC.h"
namespace app
{

	 S_WORLD_DROP_BASE* S_WORLD_DROP::findDrop(const u32 index)
	{
		if (index > maxcount) return nullptr;
		return &datas[index];
	}

	 S_WORLD_DROP_BASE* S_WORLD_DROP::findDrop_Safe(const u32 index, const s32 layer)
	{
		if (index > maxcount) return nullptr;
		if (datas[index].node.layer != layer) return nullptr;
		return &datas[index];
	}

	 // 初始化，世界的每一个格子都可能生成掉落物品，
	void S_WORLD_DROP::initData(const s32 row, const s32 col)
	{
		maxcount = row * col;
		checkpos = 0;
		this->datas = new S_WORLD_DROP_BASE[maxcount];
		for (u32 i = 0; i < maxcount; i++)
		{
			this->datas[i].Init(i);
		}
	}

	S_WORLD_DROP_BASE* S_WORLD_DROP::findDrop_Free()
	{
		if (checkpos >= maxcount) checkpos = 0;
		S_WORLD_DROP_BASE * drop = &this->datas[checkpos];
		if (!drop->isused)
		{
			checkpos++;
			return drop;
		}
		for (u32 i = checkpos + 1; i < maxcount; i++)
		{
			drop = &this->datas[checkpos];
			if (drop->isused) continue;
			checkpos = i;
			return drop;
		}
		for (u32 i = 0; i < maxcount; i++)
		{
			drop = &this->datas[checkpos];
			if (drop->isused) continue;
			checkpos = i;
			return drop;
		}
		return nullptr;
	}

	//道具掉落进入世界
	s32 S_WORLD_DROP::enterWorld(u32 mapid, S_GRID_BASE* src, s32 lockid, s32 lockteamid, s32 layer, S_ROLE_PROP* prop)
	{
		//1、寻找掉落位置
		S_WORLD_MAP* map = __World->getMap(mapid);
		bool isentry = TS_Tools::findEmptyPosNoSprite(mapid, src, layer,true);
		if (!isentry || map == nullptr)
		{
			LOG_MESSAGE("S_WORLD_DROP  error: inline:%d\n", __LINE__);
			return -1;
		}
		//2、寻找一个空闲的掉落
		S_WORLD_DROP_BASE* drop = this->findDrop_Free();
		if (drop == nullptr)
		{
			LOG_MESSAGE("S_WORLD_DROP  error: inline:%d\n", __LINE__);
			return -1;
		}
		//3、加入到节点信息
		S_GRID_BASE  biggird;
		TS_Tools::gridToMax(src, &biggird);
		bool ispush = map->PushNode(&biggird, &drop->node);
		if (!ispush)
		{
			LOG_MESSAGE("WorldMap ispush error: inline:%d\n", __LINE__);
			return -1;
		}
		drop->bc.grid_pos = src;
		drop->bc.grid_big = biggird;

		drop->lock_index = lockid;
		drop->lock_teamid = lockteamid;
		drop->lock_time = global_gametime;
		drop->isused = true;
		drop->node.layer = layer;

		map->SetNewEdge(&biggird, &drop->bc.edge);
		memcpy(&drop->prop, prop, sizeof(S_ROLE_PROP));

		map->AddSpriteCollide_Fast(src, E_SPRITE_COLLIDE_PROP, layer,3);


	//	LOG_MSG("掉落enterWorld... index:%d count:%d  [big:%d/%d] [%d/%d]\n", drop->node.index, curcount,
	//		drop->bc.grid_big.row, drop->bc.grid_big.col, biggird.row, biggird.col);

		return drop->node.index;
	}
	//掉落道具 离开世界
	bool S_WORLD_DROP::leaveWorld(u32 mapid, u32 index, s32 layer)
	{
		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr)
		{
			LOG_MESSAGE("S_WORLD_DROP::leaveWorld  error: inline:%d\n", __LINE__);
			return false;
		}
		if (index >= maxcount)
		{
			LOG_MESSAGE("S_WORLD_DROP::leaveWorld  error: inline:%d\n", __LINE__);
			return false;
		}
		S_WORLD_DROP_BASE* drop = &this->datas[index];
		if (!drop->isused)
		{
			LOG_MESSAGE("S_WORLD_DROP::leaveWorld  error: inline:%d\n", __LINE__);
			return false;
		}
		// 从世界链表中弹出
		bool ispop = map->PopNode(&drop->bc.grid_big, &drop->node);
		if (!ispop)
		{
			LOG_MESSAGE("S_WORLD_DROP::leaveWorld  error: inline:%d\n", __LINE__);
			return false;
		}

	//	LOG_MSG("掉落leaveWorld index:%d count:%d [big:%d/%d]\n", index, curcount, drop->bc.grid_big.row, drop->bc.grid_big.col);

		map->DelSpriteCollide_Fast(&drop->bc.grid_pos, E_SPRITE_COLLIDE_PROP, layer, 3);
		drop->Init(drop->node.index);
		curcount--;
		if (curcount < 0) curcount = 0;

		return true;
	}

	bool S_WORLD_DROP::createRobotDrop(u32 mapid, S_GRID_BASE* src, s32 lockid, s32 lockteamid, s32 layer, S_ROLE_PROP* prop)
	{
		S_GRID_BASE newsrc;
		newsrc = src;
		s32 dropindex = enterWorld(mapid, &newsrc, lockid, lockteamid, layer, prop);
		if (dropindex > -1)
		{
			curcount++;
			//广播掉落
			TS_Broadcast::bc_DropInfo(mapid, dropindex);
		}
		return false;
	}
	void S_WORLD_DROP::clearDrop(u32 mapid, s32 layer)
	{
		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return;
		S_RECT_BASE * edge = &map->Node.rect;
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
				S_WORLD_DROP_BASE* drop = map->Drop.findDrop_Safe(node->index, layer);
				if (drop == nullptr)
				{
					node = node->downnode;
					continue;
				}
				u32 index = node->index;
				s32 layer = node->layer;
				node = node->downnode;
				map->Drop.leaveWorld(map->mapID, index, layer);
			}
		}
	}

	//改变掉落的层
	void S_WORLD_DROP::changeLayer(u32 mapid, s32 layer, s32 newlayer)
	{
		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return;

		S_RECT_BASE * edge = &map->Node.rect;
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
					S_WORLD_DROP_BASE* drop = map->Drop.findDrop_Safe(node->index, layer);
					if (drop == nullptr)
					{
						node = node->downnode;
						continue;
					}
					drop->node.layer = newlayer;
					node = node->downnode;
				}
			}
	}
}