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
	// 1.��ͼ�����Ϣ�ļ�
	struct S_WORLD_MAP_FILE
	{
		u16						max_row;		// С����120*120
		u16						max_col;
		u8**					collides;		// �������
		bool InitData(const u32 mapid, struct S_WORLD_MAP_POS* pos);
		inline u8 getCollidesValue(u32 row, u32 col)
		{
			if (row >= MAX_MAP_ROW || row < 0) return 0;
			if (col >= MAX_MAP_COL || col < 0) return 0;
			return collides[row][col];
		}
	};

	// ����ڵ���
	struct S_WORLD_NODE_TREE
	{
		u32						count;			// ����
		S_WORLD_NODE*			rootnode;		// ���ڵ�
		S_WORLD_NODE*			lastnode;		// β�ڵ�

		void Init();
		bool Pop(S_WORLD_NODE* _node);
		bool Push(S_WORLD_NODE* _node);
	};
	// 2.�Ź���ڵ㣬ÿ���ڵ������������ݣ�trees��һ���ʹ�����һ����ͼ�������еĴ����
	struct S_WORLD_MAP_NODE
	{
		S_RECT_BASE				rect;			// ���ͼ��Χ
		S_WORLD_NODE_TREE**		trees;

		void InitData(const u16 row, const u16 col);
	};

	// ÿһ���е����ݣ�����һЩ���䡢����ȵ�
	struct S_WORLD_MAP_LAYER_BASE
	{
		s32						count;
		u8					SprCollides[MAX_MAP_ROW][MAX_MAP_COL];	// SprCollides[i][j]�ж���ʱ���ٱ������������������Ҳ������
		inline void Reset()
		{
			memset(this, 0, sizeof(S_WORLD_MAP_LAYER_BASE));
		}
	};
	// 3.�����ͼ�ϵĳ�������ر�ĵ�����
	struct S_WORLD_MAP_POS
	{
		std::vector<S_GRID_BASE*>	born;		// ������
		std::vector<S_GRID_BASE*>	reborn;		// �����
		std::vector<S_GRID_BASE*>	transmit;	// ���͵�����

		S_GRID_BASE* getBorn();
		S_GRID_BASE* getReborn();
		S_GRID_BASE* getTransmit();
	};

	// ����
	//������Ʒ
	struct S_WORLD_DROP_BC
	{
		S_GRID_BASE   grid_pos; //������Ϣ
		S_GRID_BASE   grid_big; //�������Ϣ
		S_RECT_BASE   edge;//�߽�����
		inline void reset()
		{
			memset(this, 0, sizeof(S_WORLD_DROP_BC));
		}
	};

	struct S_WORLD_DROP_BASE
	{
		bool  isused;
		s32   lock_index;     //�������ID
		s32   lock_teamid;    //��������ID
		s64   lock_time;      //����ʱ��
		s32   temp_time;

		S_WORLD_NODE     node;	// �ڵ���Ϣ
		S_ROLE_PROP      prop;	// ������ߵ�����
		S_WORLD_DROP_BC  bc;	// �㲥��Ϣ
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
		s32   curcount;// ��ǰ����
		s32   maxcount;// ��������
		u32   checkpos;// ���λ��,�����궨�ҿ��ҵ���ʲôλ��
		S_WORLD_DROP_BASE* datas;		// ���datas��initData�г�ʼ������N���������ͼ����������S_WORLD_DROP_BASE��
										// ���������ڵ�ַ�е�λ��
										// ����һ���̶���node.index = datas[index],���Ǹ��̶�ֵ��
										// ͨ���õ�dropindex���Ǿݴ˱궨
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

	// 5.Ϊ���������	
	//���1-10000 ���˸��� 10001-������ ���˸���
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
		S_VECTOR3				leftpos;		// ���Ͻǲο�����
		S_WORLD_MAP_FILE		Data;			// ��ͼ����
		S_WORLD_MAP_NODE		Node;			// �ڵ���Ϣ����ʵ�൱����һ������
												// �����е�ÿһ��������һ���������е����
		S_WORLD_MAP_POS			Pos;			// λ����Ϣ������������㣬������ȵȣ�
		S_WORLD_DROP            Drop;			// 4 ������Ϣ
		S_WORLD_MAP_LAYER		Layer;			// ����Ϣ

		bool IsRect(const u16 row, const u16 col);	// �жϽڵ��Ƿ���������������
		bool IsRect(S_GRID_BASE* pos);
		bool IsRect(S_RECT_BASE* rect);
		bool IsInGrid(S_GRID_BASE* pos);

		bool PopNode(S_GRID_BASE* pos, S_WORLD_NODE* _node);		// �Ӵ���������Ƴ�
		bool PushNode(S_GRID_BASE* pos, S_WORLD_NODE* _node);		// ������������
		void SetNewEdge(S_GRID_BASE* pos, S_RECT_BASE* rect);		// ���þŹ�����Ϣ

		bool Get_Redius_View(S_GRID_BASE* pos, s32 radius, S_RECT_BASE* rect);

		bool IsSpriteCollide_NoSprite(S_GRID_BASE* pos, s32 _layer, bool isdrop);
		void AddSpriteCollide_Fast(S_GRID_BASE* pos, u8 kind, s32 _layer, s32 id);
		void DelSpriteCollide_Fast(S_GRID_BASE* pos, u8 kind, s32 _layer, s32 id);
	};

	struct S_WORLD
	{
		S_WORLD_MAP map[MAX_MAP_ID];

		//���ճ� ���ڸ����� 
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

