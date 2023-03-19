#include "RobotData.h"
#include "WorldMap.h"
#include "WorldTools.h"

namespace app
{
	void S_ROBOT::init(const u8 kind, const u32 index)
	{
		node.init(kind, index, -1);
		bc.reset();
		data.status.reset();
		data.ai.reset();
		tmp.reset();
	}


	bool S_ROBOT::createInWorld(const s32 mapid, const s32 row, const s32 col)
	{
		if (mapid > 0) data.status.mapid = mapid;
		if (row > -1) bc.grid_pos.row = row;
		if (col > -1) bc.grid_pos.col = col;

		S_GRID_BASE newpos;
		bool isfind = findWorldCoordinate(mapid, &bc.grid_pos, &newpos);

		if (isfind)
		{
			data.status.state = E_SPRITE_STATE_LIVING;
			data.ai.think = AI_FREE;
			data.ai.base.lock_id = -1;
			data.ai.base.lock_type = N_FREE;
			bool isenter = enterWorld(mapid, &newpos);
			if (!isenter) return false;
// 			LOG_MESSAGE("robot createInWorld...index:%d [row:%d-%d] [big:%d-%d] [%d:%d %d:%d] dir:%d\n", 
// 				this->node.index, row, col, this->bc.grid_big.row, this->bc.grid_big.col,this->bc.edge.left, 
// 				this->bc.edge.top, this->bc.edge.right, this->bc.edge.bottom,this->data.status.dir);
		}
		return isfind;
	}


	bool S_ROBOT::enterWorld(const s32 mapid, S_GRID_BASE* pos)
	{
		if (bc.isinWorld) return false;
		S_WORLD_MAP* map = __World->getMap(mapid);

		if (map == nullptr)                    return false;
		if (tmp.script == nullptr)             return false;
		if (tmp.script->id != data.status.id)  return false;

		//设置地图ID和坐标信息
		data.status.mapid = mapid;
		bc.grid_pos = pos;

		// 设置移动边界
		u32 range = tmp.script->range;
		data.status.move_edge.left = pos->col - range;
		data.status.move_edge.right = pos->col + range;
		data.status.move_edge.top = pos->row - range;
		data.status.move_edge.bottom = pos->row + range;
		if (data.status.move_edge.left < 0)   data.status.move_edge.left = 0;
		if (data.status.move_edge.top < 0)    data.status.move_edge.top = 0;
		if (data.status.move_edge.right < 0)  data.status.move_edge.right = 0;
		if (data.status.move_edge.bottom < 0) data.status.move_edge.bottom = 0;

		if (data.status.move_edge.bottom >= map->Data.max_row)  data.status.move_edge.bottom = map->Data.max_row - 1;
		if (data.status.move_edge.right >= map->Data.max_col) data.status.move_edge.right = map->Data.max_col - 1;

		//LOG_MSG("怪物move_edge:index:%d row:%d/%d  col:%d/%d born:%d/%d\n",this->node.index, 
		//	data.status.move_edge.top, data.status.move_edge.bottom,
		//	data.status.move_edge.left, data.status.move_edge.right,pos->row,pos->col);
		//获得当前大格子区域
		TS_Tools::gridToMax(pos, &bc.grid_big);
		map->SetNewEdge(&bc.grid_big, &bc.edge);
		map->PushNode(&bc.grid_big, &node);
		// 在地图中添加添加这个怪物的占有格子信息，因为在createworld中调用的时候，已经确定这个位置是可以放机器人的
		map->AddSpriteCollide_Fast(&bc.grid_pos, getSpriteCollideType(), this->node.layer, 1);
		bc.isinWorld = true;

		//	LOG_MSG("robot enterWorld..index:%d layer:%d \n", this->node.index,this->node.layer);
		return true;
	}


	bool S_ROBOT::leaveWorld()
	{
		if (!bc.isinWorld)
		{
			LOG_MESSAGE("Robot leaveWorld err...%d, line:%d\n", this->node.layer, __LINE__);
			return false;
		}
		S_WORLD_MAP* map = __World->getMap(this->data.status.mapid);
		if (map == nullptr)
		{
			LOG_MESSAGE("Robot leaveWorld err...%d, mapid=%d line:%d\n", this->node.layer, this->data.status.mapid, __LINE__);
			return false;
		}
		LOG_MESSAGE("Robot leaveWorld...%d, mapid=%d line:%d\n", this->node.layer, this->data.status.mapid, __LINE__);

		map->DelSpriteCollide_Fast(&this->bc.grid_pos, getSpriteCollideType(), this->node.layer, 1);
		map->PopNode(&bc.grid_big, &node);

		bc.isinWorld = false;
		return true;
	}



	bool S_ROBOT::findWorldCoordinate(const s32 mapid, S_GRID_BASE* curpos, S_GRID_BASE* newpos)
	{
		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return false;

		bool isgrid = map->IsInGrid(curpos);
		bool isno = map->IsSpriteCollide_NoSprite(curpos, this->node.layer, false);
		if (isgrid && isno)
		{
			*newpos = curpos;
			return true;
		}
		// curpos这个点被其他占领了，需要一个新得位置。就从curpos的周围九格找一个空闲，找到了返回T，没有则F
		*newpos = curpos;
		bool isnoxx = TS_Tools::findEmptyPosNoSprite(mapid, newpos, this->node.layer);

		return isnoxx;
	}


	bool S_ROBOT::moveWorld(u32 row, u32 col, u8 dir)
	{
		S_WORLD_MAP* map = __World->getMap(this->data.status.mapid);
		if (map == nullptr)
		{
			LOG_MESSAGE("robot moveworld err... %d-%d line:%d",
				this->node.index, this->data.status.mapid, __LINE__);
			return false;
		}

		if (this->data.status.hp <= 0)
		{
			LOG_MESSAGE("robot moveworld err hp < 0... %d-%d line:%d",
				this->node.index, this->data.status.mapid, __LINE__);
			return false;
		}

		S_GRID_BASE tmpgrid;
		tmpgrid.row = row;
		tmpgrid.col = col;

		// 判断是否换了大区域
		S_GRID_BASE big_pos;
		TS_Tools::gridToMax(&tmpgrid, &big_pos);


		if (bc.grid_big.row != big_pos.row || bc.grid_big.col != big_pos.col)
		{
			map->PopNode(&bc.grid_big, &node);
			map->PushNode(&big_pos, &node);
			map->SetNewEdge(&big_pos, &bc.edge);
		}
		map->DelSpriteCollide_Fast(&this->bc.grid_pos, getSpriteCollideType(), this->node.layer, 2);
		this->data.status.dir = dir;
		this->bc.grid_pos = tmpgrid;
		this->bc.grid_big = big_pos;
		map->AddSpriteCollide_Fast(&tmpgrid, getSpriteCollideType(), this->node.layer, 2);
		return true;
	}
}