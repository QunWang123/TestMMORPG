#include "WorldMap.h"
#include <io.h>
#include <time.h>
#include "ShareFunction.h"

namespace app
{
	S_WORLD* __World = new S_WORLD();
	char tempMapData[MAX_MAP_LENGTH];

	// 加载地图数据
	bool S_WORLD::LoadMap()
	{
		initLayerPool();

		for (u32 i = 1; i < MAX_MAP_ID; i++)
		{
			//1 检查地图启用
			u8 value = share::__MapUsingList[i];
			if (value != 1) continue;

			S_WORLD_MAP* map = &this->map[i];
			bool isload = map->Data.InitData(i, &map->Pos);
			if (isload == false)
			{
				LOG_MESSAGE("[error] map load failed...%d \n", i);
				continue;
			}

			map->isUsed = true;
			map->mapID = i;

			// 初始化世界节点数
			map->Node.InitData(map->Data.max_row, map->Data.max_col);
			map->Drop.initData(map->Data.max_row, map->Data.max_col);
			map->Layer.InitData(map->Data.max_row, map->Data.max_col);
		}
		return true;
	}

	bool S_WORLD_MAP_FILE::InitData(const u32 mapid, S_WORLD_MAP_POS* pos)
	{
		if (mapid >= MAX_MAP_ID) return false;
		S_WORLD_MAP* map = &__World->map[mapid];

		char fname[1000];
		char filename[200];
		sprintf_s(filename, "%d.bin", mapid);
		memset(fname, 0, 1000);

		sprintf_s(fname, "%s//bin//%s", func::FileExePath, filename);
		int isexist = _access(fname, 0);
		if (isexist == -1) return false;

		int m_row = 0;
		int m_col = 0;

		FILE * m_File = _fsopen(fname, "rb+", _SH_DENYNO);
		if (m_File == nullptr) return false;
		fseek(m_File, 0, SEEK_SET);
		fread(&map->leftpos, 1, 12, m_File);
		// 便宜到12 读取最大值
		fseek(m_File, 12, SEEK_SET);
		fread(&m_row, 1, 4, m_File);
		// 便宜到16 读取最大值
		fseek(m_File, 16, SEEK_SET);
		fread(&m_col, 1, 4, m_File);

		if (m_row >= MAX_MAP_ROW || m_row < 0) return false;
		if (m_col >= MAX_MAP_COL || m_col < 0) return false;

		// 这个大小根据实际去调，60*60个小格子，任意10*10为一个大格子
		this->max_row = m_row;
		this->max_col = m_col;

		this->collides = new u8*[max_row];
		for (int i = 0; i < this->max_row; i++)
		{
			this->collides[i] = new u8[max_col];
			for (int j = 0; j < this->max_col; j++)
			{
				this->collides[i][j] = 0;
			}
		}

		int readBytes = this->max_row * this->max_col;
		memset(tempMapData, 0, MAX_MAP_LENGTH);
		fseek(m_File, 20, SEEK_SET);
		fread(&tempMapData, 1, readBytes, m_File);

		for (int i = 0; i < readBytes; i++)
		{
			int row = i / this->max_row;
			int col = i % this->max_row;
			u8 value = tempMapData[i];
			if (row == max_row)
			{
				break;
			}
			this->collides[row][col] = value;

			switch (value)
			{
			case EGTY_BORN:
			{
				S_GRID_BASE* g = new S_GRID_BASE();
				g->row = row;
				g->col = col;
				pos->born.push_back(g);
			}
				break;
			case EGTY_REBORN:
			{
				S_GRID_BASE* g = new S_GRID_BASE();
				g->row = row;
				g->col = col;
				pos->reborn.push_back(g);
			}
				break;
			case EGTY_TRANSMIT:
			{
				S_GRID_BASE* g = new S_GRID_BASE();
				g->row = row;
				g->col = col;
				pos->transmit.push_back(g);
			}
				break;
			}
		}
		return true;
	}

	// 设置世界地图范围，这个是大格子坐标
	void S_WORLD_MAP_NODE::InitData(const u16 row, const u16 col)
	{
		this->rect.left = 0;
		this->rect.top = 0;
		this->rect.right = col / C_WORLDMAP_NODE_Y + 1;
		this->rect.bottom = row / C_WORLDMAP_NODE_X + 1;

		this->trees = new S_WORLD_NODE_TREE*[this->rect.bottom];
		for (int i = 0; i < this->rect.bottom; i++)
		{
			this->trees[i] = new S_WORLD_NODE_TREE[this->rect.right];
			for (int j = 0; j < this->rect.right; j++)
			{
				this->trees[i][j].Init();
			}
		}
	}

	void S_WORLD_NODE_TREE::Init()
	{
		count = 0;
		rootnode = nullptr;
		lastnode = nullptr;
	}

	bool S_WORLD_NODE_TREE::Pop(S_WORLD_NODE* _node)
	{
		if (_node == nullptr)
		{
			LOG_MESSAGE("World Map pop err...line:%d\n", __LINE__);
			return false;
		}

		if (rootnode == nullptr || lastnode == nullptr)
		{
			LOG_MESSAGE("World Map pop err...line:%d\n", __LINE__);
			return false;
		}

		S_WORLD_NODE* up = _node->upnode;
		S_WORLD_NODE* down = _node->downnode;

		if (rootnode == _node)
		{
			rootnode = down;
			if(rootnode != nullptr) rootnode->upnode = nullptr;
			if (_node == lastnode) lastnode = rootnode;
		}
		else
		{
			if (down != nullptr) down->upnode = up;
			if (up != nullptr) up->downnode = down;
			if (_node == lastnode) lastnode = up;
		}

		_node->upnode = nullptr;
		_node->downnode = nullptr;
		count -= 1;
		return true;
	}

	bool S_WORLD_NODE_TREE::Push(S_WORLD_NODE* _node)
	{
		if (_node == nullptr || (_node->upnode != nullptr || _node->downnode != nullptr))
		{
			LOG_MESSAGE("World Map push err...line:%d\n", __LINE__);
			return false;
		}

		if (rootnode == nullptr)
		{
			rootnode = _node;
			lastnode = _node;
			rootnode->downnode = nullptr;
			rootnode->upnode = nullptr;
		}
		else
		{
			if (_node == lastnode)
			{
				LOG_MESSAGE("World Map push self err...line:%d\n", __LINE__);
				return false;
			}
			_node->upnode = lastnode;
			lastnode->downnode = _node;
			lastnode = _node;
		}
		count += 1;
		return true;
	}






	bool S_WORLD_MAP::IsRect(const u16 row, const u16 col)
	{
		if (col >= this->Node.rect.right) return false;
		if (row >= this->Node.rect.bottom) return false;
		return true;
	}

	bool S_WORLD_MAP::IsRect(S_GRID_BASE* pos)
	{
		if (pos == nullptr ||
			pos->row < 0 || pos->col < 0 ||
			pos->col >= this->Node.rect.right ||
			pos->row >= this->Node.rect.bottom) return false;
		return true;
	}

	bool S_WORLD_MAP::IsRect(S_RECT_BASE* rect)
	{
		if (rect->left < 0) rect->left = 0;
		if (rect->top < 0) rect->top = 0;
		if (rect->right >= this->Node.rect.right) rect->right = this->Node.rect.right - 1;
		if (rect->bottom >= this->Node.rect.bottom) rect->bottom = this->Node.rect.bottom - 1;

		if (rect->right < 0)
		{
			rect->right = 0;
			LOG_MESSAGE("World Map isrect1 err %d-%d-%d-%d...line:%d\n", 
				rect->left, rect->right, rect->top, rect->bottom, __LINE__);
			return false;
		}

		if (rect->bottom < 0)
		{
			rect->bottom = 0;
			LOG_MESSAGE("World Map isrect2 err %d-%d-%d-%d...line:%d\n",
				rect->left, rect->right, rect->top, rect->bottom, __LINE__);
			return false;
		}

		return true;
	}

	// 格子坐标是否在世界坐标范围内
	bool S_WORLD_MAP::IsInGrid(S_GRID_BASE* pos)
	{
		if (pos == nullptr ||
			pos->row < 0 || pos->col < 0 ||
			pos->col >= this->Data.max_col ||
			pos->row >= this->Data.max_row) return false;
		return true;
	}
	// 将S_WORLD_NODE插入链表。将它的格子S_GRID_BASE传入函数，pos在地图里则插入_node
	bool S_WORLD_MAP::PopNode(S_GRID_BASE* pos, S_WORLD_NODE* _node)
	{
		if (!IsInGrid(pos))
		{
			LOG_MESSAGE("World Map pop err ...line:%d\n", __LINE__);
			return false;
		}
		this->Node.trees[pos->row][pos->col].Pop(_node);
		return true;
	}

	bool S_WORLD_MAP::PushNode(S_GRID_BASE* pos, S_WORLD_NODE* _node)
	{
		if (!IsInGrid(pos))
		{
			LOG_MESSAGE("World Map push err ...line:%d\n", __LINE__);
			return false;
		}
		this->Node.trees[pos->row][pos->col].Push(_node);
		return true;
	}
	// 设置九宫格边界数据，对每一个S_GRID_BASE都来一次
	void S_WORLD_MAP::SetNewEdge(S_GRID_BASE* pos, S_RECT_BASE* rect)
	{
		if (pos == nullptr) return;
		rect->left = pos->col - 1;
		rect->right = pos->col + 1;

		rect->top = pos->row - 1;
		rect->bottom = pos->row + 1;

		if (rect->left < 0) rect->left = 0;
		if (rect->top < 0) rect->top = 0;
		if (rect->right >= this->Node.rect.right) rect->right = this->Node.rect.right - 1;
		if (rect->bottom >= this->Node.rect.bottom) rect->bottom = this->Node.rect.bottom - 1;
	
		if (rect->right < 0)
		{
			rect->right = 0;
			LOG_MESSAGE("World Map SetNewEdge err1 %d-%d-%d-%d...line:%d\n",
				rect->left, rect->right, rect->top, rect->bottom, __LINE__);
		}

		if (rect->bottom < 0)
		{
			rect->bottom = 0;
			LOG_MESSAGE("World Map SetNewEdge err2 %d-%d-%d-%d...line:%d\n",
				rect->left, rect->right, rect->top, rect->bottom, __LINE__);
		}

// 		LOG_MESSAGE("World Map SetNewEdge success [%d-%d] [%d-%d] [%d-%d]...line:%d\n",
// 			pos->row, pos->col, rect->top, rect->left, rect->bottom, rect->right, __LINE__);
	}
	// 根据格子坐标生成大区域
	bool S_WORLD_MAP::Get_Redius_View(S_GRID_BASE* pos, s32 radius, S_RECT_BASE* rect)
	{
		rect->left = (pos->col - radius) / C_WORLDMAP_NODE_Y;
		rect->top = (pos->row - radius) / C_WORLDMAP_NODE_X;
		rect->right = (pos->col + radius) / C_WORLDMAP_NODE_Y;
		rect->bottom = (pos->row + radius) / C_WORLDMAP_NODE_X;

		if (rect->left < 0) rect->left = 0;
		if (rect->top < 0) rect->top = 0;

		if (rect->right < 0 || rect->bottom < 0)
		{
			return false;
		}

		if (rect->right >= this->Node.rect.right) rect->right = this->Node.rect.right - 1;
		if (rect->bottom >= this->Node.rect.bottom) rect->bottom = this->Node.rect.bottom - 1;
		return true;
	}

	app::S_WORLD_MAP* S_WORLD::getMap(u32 mapid)
	{
		if (mapid >= MAX_MAP_ID ||
			!this->map[mapid].isUsed) return nullptr;
		return &this->map[mapid];
	}

	void S_WORLD::initLayerPool()
	{
		for (int count = 0; count < MAX_LAYER_POOL; count++)
		{
			S_WORLD_MAP_LAYER_BASE* ly = new S_WORLD_MAP_LAYER_BASE();
			ly->Reset();
			this->Layer_Pool.push(ly);
		}

		LOG_MESSAGE("initLayerPool...%lld \n", Layer_Pool.size());
	}

	void S_WORLD::pushLayerPool(S_WORLD_MAP_LAYER_BASE* ly)
	{
		if (ly == nullptr) return;
		ly->Reset();
		Layer_Pool.push(ly);
	}
	S_WORLD_MAP_LAYER_BASE* S_WORLD::popLayerPool()
	{
		S_WORLD_MAP_LAYER_BASE* ly = nullptr;
		if (Layer_Pool.empty())
		{
			ly = new S_WORLD_MAP_LAYER_BASE();
			ly->Reset();
		}
		else
		{
			ly = Layer_Pool.front();
			Layer_Pool.pop();
			ly->Reset();
		}
		return ly;
	}

	app::S_GRID_BASE* S_WORLD_MAP_POS::getBorn()
	{
		srand((int)time(0));		// 传参不固定即可实现产生不同得随机数序列

		u8 index = rand() % born.size();
		return index < born.size() ? born[index] : nullptr;
	}

	app::S_GRID_BASE* S_WORLD_MAP_POS::getReborn()
	{
		srand((int)time(0));

		u8 index = rand() % reborn.size();
		return index < reborn.size() ? reborn[index] : nullptr;
	}

	app::S_GRID_BASE* S_WORLD_MAP_POS::getTransmit()
	{
		srand((int)time(0));

		u8 index = rand() % transmit.size();
		return index < transmit.size() ? transmit[index] : nullptr;
	}

	//初始化层
	void S_WORLD_MAP_LAYER::InitData(const u16 row, const u16 col)
	{
		//每张图只初始化1层 其他的动态来获取
		for (u32 layer = 0; layer < MAX_LAYER_COUNT; layer++)
		{
			this->sprtype[layer] = nullptr;
			if (layer >= MIN_LAYER_COUNT) continue;

			S_WORLD_MAP_LAYER_BASE* ly = new S_WORLD_MAP_LAYER_BASE();
			ly->Reset();
			this->sprtype[layer] = ly;
		}
	}

	//设置一个层级数据 主要用于填充怪物或者角色 掉落的格子信息
	//避免怪物 角色 掉落 重叠在一起
	//只有副本才会设置 一般普通地图不需要
	void S_WORLD_MAP_LAYER::setLayerType(const s32 layer)
	{
		if (layer < 0 || layer >= MAX_LAYER_COUNT) return;

		sprtype[layer] = __World->popLayerPool();
	}

	//使用完毕 放回回收池 
	void S_WORLD_MAP_LAYER::clearLayerType(const s32 layer)
	{
		if (layer < 0 || layer >= MAX_LAYER_COUNT) return;

		auto d = sprtype[layer];
		if (d == nullptr) return;

		__World->pushLayerPool(d);
		sprtype[layer] = nullptr;
	}

	bool S_WORLD_MAP::IsSpriteCollide_NoSprite(S_GRID_BASE* pos, s32 _layer, bool isdrop)
	{
		if (!IsInGrid(pos))  return false;
		if (_layer >= MAX_ROOM_COUNT) return false;
		s32 layer = _layer;
		if (layer < 0) layer = 0;

		auto ly = this->Layer.sprtype[layer];
		if (ly == nullptr) return false;

		u8 value = Data.collides[pos->row][pos->col];
		u8 type = ly->SprCollides[pos->row][pos->col];
		//验证这个坐标点是不是阻隔点 
		//验证这个坐标点上 有没有其他类型的？怪物只能独立占用
		// isdrop就是确定掉落物是否可以独占一个格子
		if (isdrop == false)
		{
			if (value != EGTY_STOP && (type == E_SPRITE_COLLIDE_FREE || type == E_SPRITE_COLLIDE_PROP))
			{
				return true;
			}
		}
		else
		{
			if (value != EGTY_STOP && ((type & E_SPRITE_COLLIDE_PROP) == 0))
			{
				return true;
			}
		}
		return false;
	}

	void S_WORLD_MAP::AddSpriteCollide_Fast(S_GRID_BASE* pos, u8 kind, s32 _layer, s32 id)
	{
		if (!IsInGrid(pos)) return;
		if (_layer >= MAX_ROOM_COUNT) return;
		s32 layer = _layer;
		if (layer < 0) layer = 0;

		auto ly = this->Layer.sprtype[layer];
		if (ly == nullptr) return;

		if ((ly->SprCollides[pos->row][pos->col] & kind) != E_SPRITE_COLLIDE_FREE)
		{
			LOG_MESSAGE("err AddSpriteCollide_Fast..layer:%d kind:%d [%d-%d]. %d inline:%d\n",
				layer, kind, pos->row, pos->col, id, __LINE__);
			return;
		}
		ly->SprCollides[pos->row][pos->col] |= kind;
		//	LOG_MSG("AddSpriteCollide_Fast..layer:%d [%d-%d]  %d inline:%d\n", layer, pos->row, pos->col, id, __LINE__);
	}

	void S_WORLD_MAP::DelSpriteCollide_Fast(S_GRID_BASE* pos, u8 kind, s32 _layer, s32 id)
	{
		if (!IsInGrid(pos)) return;
		if (_layer >= MAX_ROOM_COUNT) return;
		s32 layer = _layer;
		if (layer < 0) layer = 0;

		auto ly = this->Layer.sprtype[layer];
		if (ly == nullptr) return;

		u8 TempCollider = ly->SprCollides[pos->row][pos->col];
		if ((TempCollider & kind) == E_SPRITE_COLLIDE_FREE)
		{
			LOG_MESSAGE("err DelSpriteCollide_Fast..layer:%d kind:%d [%d-%d]. %d inline:%d\n", 
				layer, kind, pos->row, pos->col, id, __LINE__);
			return;
		}
		ly->SprCollides[pos->row][pos->col] = TempCollider & (~kind);

		//LOG_MSG("DelSpriteCollide_Fast..layer:%d [%d-%d] .%d inline:%d\n", layer, pos->row, pos->col, id, __LINE__);
	}
}




