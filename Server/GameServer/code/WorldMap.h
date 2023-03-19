#ifndef __WORLDMAP_H
#define __WORLDMAP_H

#include "WorldData.h"
#include <vector>
#include <queue>

#define MAX_MAP_ROW 1000
#define MAX_MAP_COL 1000
#define MAX_MAP_LENGTH 1024*1024*5
#define MAX_LAYER_POOL 200

namespace app
{
	// 1.地图阻隔信息文件
	struct S_WORLD_MAP_FILE
	{
		u16						max_row;		// 小格子120*120
		u16						max_col;
		u8**					collides;		// 阻隔数据
		bool InitData(const u32 mapid, struct S_WORLD_MAP_POS* pos);
		inline u8 getCollidesValue(u32 row, u32 col)
		{
			if (row >= MAX_MAP_ROW || row < 0) return 0;
			if (col >= MAX_MAP_COL || col < 0) return 0;
			return collides[row][col];
		}
	};

	// 世界节点树
	struct S_WORLD_NODE_TREE
	{
		u32						count;			// 数量
		S_WORLD_NODE*			rootnode;		// 根节点
		S_WORLD_NODE*			lastnode;		// 尾节点

		void Init();
		bool Pop(S_WORLD_NODE* _node);
		bool Push(S_WORLD_NODE* _node);
	};
	// 2.九宫格节点，每个节点里存入玩家数据，trees这一个就代表了一个地图里面所有的大格子
	struct S_WORLD_MAP_NODE
	{
		S_RECT_BASE				rect;			// 大地图范围
		S_WORLD_NODE_TREE**		trees;

		void InitData(const u16 row, const u16 col);
	};

	// 每一层中得数据，定义一些掉落、怪物等等
	struct S_WORLD_MAP_LAYER_BASE
	{
		s32						count;
		u8					SprCollides[MAX_MAP_ROW][MAX_MAP_COL];	// SprCollides[i][j]有东西时，再爆东西不会在这里；怪物也会绕行
		inline void Reset()
		{
			memset(this, 0, sizeof(S_WORLD_MAP_LAYER_BASE));
		}
	};
	// 3.世界地图上的出生点等特别的点坐标
	struct S_WORLD_MAP_POS
	{
		std::vector<S_GRID_BASE*>	born;		// 出生点
		std::vector<S_GRID_BASE*>	reborn;		// 复活点
		std::vector<S_GRID_BASE*>	transmit;	// 传送点坐标

		S_GRID_BASE* getBorn();
		S_GRID_BASE* getReborn();
		S_GRID_BASE* getTransmit();
	};

	// 掉落
	//掉落物品
	struct S_WORLD_DROP_BC
	{
		S_GRID_BASE   grid_pos; //坐标信息
		S_GRID_BASE   grid_big; //大格子信息
		S_RECT_BASE   edge;//边界区域
		inline void reset()
		{
			memset(this, 0, sizeof(S_WORLD_DROP_BC));
		}
	};

	struct S_WORLD_DROP_BASE
	{
		bool  isused;
		s32   lock_index;     //归属玩家ID
		s32   lock_teamid;    //归属队伍ID
		s64   lock_time;      //归属时间
		s32   temp_time;

		S_WORLD_NODE     node;	// 节点信息
		S_ROLE_PROP      prop;	// 掉落道具的属性
		S_WORLD_DROP_BC  bc;	// 广播信息
		inline void Init(int index)
		{
			isused = false;
			lock_index = -1;
			lock_teamid = -1;
			temp_time = 0;
			node.init(N_PROP, index, -1);
			prop.reset();
			bc.reset();
		}
	};
	struct S_WORLD_DROP
	{
		s32   curcount;// 当前数量
		s32   maxcount;// 掉落数量
		u32   checkpos;// 检查位置,用来标定找空找到了什么位置
		S_WORLD_DROP_BASE* datas;		// 这个datas在initData中初始化，开N多个（即地图格子数个）S_WORLD_DROP_BASE，
										// 根据他们在地址中的位置
										// 给定一个固定的node.index = datas[index],这是个固定值，
										// 通信用的dropindex就是据此标定
		void initData(const s32 row, const s32 col);
		void changeLayer(u32 mapid, s32 layer, s32 newlayer);
		s32  enterWorld(u32 mapid, S_GRID_BASE* src, s32 lockid, s32 lockteamid, s32 layer, S_ROLE_PROP* prop);
		bool leaveWorld(u32 mapid, u32 index, s32 layer);
		bool createRobotDrop(u32 mapid, S_GRID_BASE* src, s32 lockid, s32 lockteamid, s32 layer, S_ROLE_PROP* prop);
		void clearDrop(u32 mapid, s32 layer);

		S_WORLD_DROP_BASE* findDrop_Free();
		S_WORLD_DROP_BASE* findDrop(const u32 index);
		S_WORLD_DROP_BASE* findDrop_Safe(const u32 index, const s32 layer);
	};

	// 5.为副本定义的	
	//玩家1-10000 单人副本 10001-最大队伍 多人副本
	struct S_WORLD_MAP_LAYER
	{
		S_WORLD_MAP_LAYER_BASE*	sprtype[MAX_LAYER_COUNT];
		
		void InitData(const u16 row, const u16 col);
		void setLayerType(const s32 layer);
		void clearLayerType(const s32 layer);
	};

	struct S_WORLD_MAP
	{
		bool					isUsed;
		u32						mapID;
		S_VECTOR3				leftpos;		// 左上角参考坐标
		S_WORLD_MAP_FILE		Data;			// 地图数据
		S_WORLD_MAP_NODE		Node;			// 节点信息，其实相当于是一个链表，
												// 链表中得每一条链代表一个大区域中得玩家
		S_WORLD_MAP_POS			Pos;			// 位置信息，（比如出生点，死亡点等等）
		S_WORLD_DROP            Drop;			// 4 掉落信息
		S_WORLD_MAP_LAYER		Layer;			// 层信息

		bool IsRect(const u16 row, const u16 col);	// 判断节点是否在世界坐标以内
		bool IsRect(S_GRID_BASE* pos);
		bool IsRect(S_RECT_BASE* rect);
		bool IsInGrid(S_GRID_BASE* pos);

		bool PopNode(S_GRID_BASE* pos, S_WORLD_NODE* _node);		// 从大格子容器移除
		bool PushNode(S_GRID_BASE* pos, S_WORLD_NODE* _node);		// 填入大格子容器
		void SetNewEdge(S_GRID_BASE* pos, S_RECT_BASE* rect);		// 设置九宫格信息

		bool Get_Redius_View(S_GRID_BASE* pos, s32 radius, S_RECT_BASE* rect);

		bool IsSpriteCollide_NoSprite(S_GRID_BASE* pos, s32 _layer, bool isdrop);
		void AddSpriteCollide_Fast(S_GRID_BASE* pos, u8 kind, s32 _layer, s32 id);
		void DelSpriteCollide_Fast(S_GRID_BASE* pos, u8 kind, s32 _layer, s32 id);
	};

	struct S_WORLD
	{
		S_WORLD_MAP map[MAX_MAP_ID];

		//回收池 用于副本层 
		std::queue<S_WORLD_MAP_LAYER_BASE*>  Layer_Pool;

		bool LoadMap();
		S_WORLD_MAP* getMap(u32 mapid);
		void  initLayerPool();
		void pushLayerPool(S_WORLD_MAP_LAYER_BASE* ly);
		S_WORLD_MAP_LAYER_BASE* popLayerPool();
	};

	extern S_WORLD* __World;
}

#endif

