#ifndef __WORLDDATA_H
#define __WORLDDATA_H
#include "INetBase.h"
#include "UserData.h"

#define C_WORLDMAP_ONE_GRID	100		// 100cm一个格子
#define C_WORLDMAP_NODE_X	10		// 一个大格子10m，这个可以根据实际情况调，屏幕大完全可以调大
#define C_WORLDMAP_NODE_Y	10		// 
#define USER_SKILL_PUBLICTIME 300
#define C_NEW_ROLE_PROTECT_LEVEL 1
#define MAX_SPRITE_COUNT	20000	// 最大数目（即机器人（各种宠物，怪物的和））
#define MAX_DIR				8
#define MAX_REQUEST_ROLE_COUNT  5

namespace app
{
	enum E_NODE_TYPE
	{
		N_FREE = 0x00,
		N_ROLE,
		N_MONSTER,
		N_NPC,
		N_PET,
		N_PROP
	};

	enum E_SPRITE_STATE
	{
		E_SPRITE_STATE_FREE = 0x00,		// 未知释放的
		E_SPRITE_STATE_LIVING,			// 活着
		E_SPRITE_STATE_BUFFING,			// BUFFER中
		E_SPRITE_STATE_PICKING,			// 采集中	
		E_SPRITE_STATE_DEAD,			// 死亡
		E_SPRITE_STATE_OUTWORLD			// 不在世界中
	};

	enum E_SKILL_USER_TYPE
	{
		E_SKILL_USE_TYPE_FREE = 0x00,	
		E_SKILL_USE_TYPE_SELF,			// 自身为目标，360度技能
		E_SKILL_USE_TYPE_DIR,			// 打方向技能
		E_SKILL_USE_TYPE_TARGET,		// 以目标点360度的技能	
		E_SKILL_USE_TYPE_HALF,			// 半圆技能
		E_SKILL_USE_TYPE_BUFF			// 直接生成buff的技能
	};

	enum E_BUFF_TYPE
	{
		E_BUFF_FREE = 0x00,
		E_BUFF_NOMOVE = 0x01,
		E_BUFF_NOSKILL = 0x02,
		E_BUFF_NOEYE = 0x04,
		E_BUFF_DELHP = 0x08,
		E_BUFF_ADDHP = 0x10,
		E_BUFF_ADDDEFEND = 0x20,
		E_BUFF_ADDATK = 0x40,
	};

	enum E_GRIDTYPE_INFO
	{
		EGTY_STOP = 0x00,
		EGTY_RUN = 0x01,
		EGTY_BORN = 0x02,
		EGTY_REBORN = 0x03,
		EGTY_TRANSMIT = 0x04,
		EGTY_ADDHP = 0x10,
		EGTY_ADDDEFEND = 0x20,
		EGTY_ADDATK = 0x40,
	};

	enum E_SPRITE_COLLIDE
	{
		E_SPRITE_COLLIDE_FREE = 0x00,
		E_SPRITE_COLLIDE_MONSTER = 0x01,
		E_SPRITE_COLLIDE_NPC = 0x02,
		E_SPRITE_COLLIDE_PET = 0x04,
		E_SPRITE_COLLIDE_ROLE = 0x08,
		E_SPRITE_COLLIDE_PROP = 0x10
	};
	enum E_CHANGEMAP_KIND
	{
		ECK_LEAVE_NO = 0x00, //切换地图 不清理
		ECK_LEAVE_YES = 0x01,//切换地图清理
		ECK_SUCCESS = 0x02    //切换地图 成功
	};

	enum E_MAP_TYPE
	{
		EMT_PUBLIC_MAP = 0x00,  //公共地图
		EMT_COPY_ONE = 0x01,    //个人副本
		EMT_COPY_MORE = 0x02    //多人副本
	};
#pragma pack(push, packing)
#pragma pack(1)
	// 临时数据

	// 技能
	struct S_TEMP_SKILL_RUN_BASE
	{
		this_constructor(S_TEMP_SKILL_RUN_BASE);

		// u8		index;
		s32		skill_id;		// 技能ID
		u8		skill_index;	// 当前运行技能的下标
		u8		skill_level;	// 技能等级
		s32		continued;		// 技能持续时间
		s32		runingtime;		// 运行时间
		s32		flytime;		// 技能飞行时间
		s32		damagecount;	// 伤害段数，1是一下 

		S_VECTOR3	targetpos;	// 目标点
		s32		lock_index;		// 锁定玩家
		u8		lock_type;		// 锁定类型

		s32		continued_waittime;	// 等待多久
		s32		temp_continuedtime;	// 记录技能运行时间
		S_VECTOR3	temp_calpos;	// 记录冲刺/打方向技能前所在的位置

		inline void reset()
		{
			memset(this, 0, sizeof(S_TEMP_SKILL_RUN_BASE));
		}
	};

	// 正在运行中的技能
	struct S_TEMP_SKILL_RUN
	{
		this_constructor(S_TEMP_SKILL_RUN);
		u64			runtime;	// 技能运行时间
		S_TEMP_SKILL_RUN_BASE	data[MAX_SKILL_COUNT];
		inline void reset()
		{
			memset(this, 0, sizeof(S_TEMP_SKILL_RUN));
		}
		inline S_TEMP_SKILL_RUN_BASE* findFree()
		{
			for (u32 i = 0; i < MAX_SKILL_COUNT; i++)
			{
				if(this->data[i].skill_id > 0) continue;
				this->data[i].reset();
				// this->data[i].index = i;
				return &this->data[i];
			}
			return nullptr;
		}
	};

	struct S_TEMP_BUFF_RUN_BASE
	{
		this_constructor(S_TEMP_BUFF_RUN_BASE);
		s32			buff_id;
		s32			runningtime;
		u64			temptime;
		inline void reset()
		{
			memset(this, 0, sizeof(S_TEMP_BUFF_RUN_BASE));
		}
	};

	// 正在运行中的BUFF
	struct S_TEMP_BUFF_RUN
	{
		this_constructor(S_TEMP_BUFF_RUN);
		s32			state;
		S_TEMP_BUFF_RUN_BASE	data[MAX_BUFF_COUNT];
		inline void reset()
		{
			memset(this, 0, sizeof(S_TEMP_BUFF_RUN));
		}
		inline S_TEMP_BUFF_RUN_BASE* findBuff(s32 index)
		{
			if (index < 0 || index >= MAX_BUFF_COUNT) return nullptr;
			return &data[index];
		}
		inline S_TEMP_BUFF_RUN_BASE* findFree()
		{
			for (s32 i = 0; i < MAX_BUFF_COUNT; i++)
			{
				if (data[i].buff_id <= 0)
				{
					data[i].reset();
					return &data[i];
				}
			}
			return nullptr;
		}
		inline bool isMove()
		{
			s32 value = state & E_BUFF_NOMOVE;
			if (value > 0) return false;
			return true;
		}
		inline bool isUseSkill()
		{
			s32 value = state & E_BUFF_NOSKILL;
			if (value > 0) return false;
			return true;
		}
		inline void	addState(s32 value)
		{
			state = state | value;
		}
		inline void	removeState(s32 value)
		{
			// 0000 0011	state
			// 0000 0001	value
			// 0000 0010	异或

			// 0000 0010	state
			// 0000 0001	value		1111 1110 取反
			// 0000 0011	异或
			// 0000 0000	按位与取反	更好

			state = state & (~value);
			// state = state ^ value;
		}
	};

	struct S_USER_TEMP_GAME
	{
		bool    bChangeMap;
		s32		userindex;				// DB在线玩家索引
		s32		user_connectid;			// 玩家在网关上的id
		s32		server_connectid;		// 玩家连接的网关服务器的ID
		s32		server_clientid;		

// 		u8		temp_GamePage;			// 当前玩家所在界面
// 		s32		temp_CopyIndex;			// 副本房间索引
// 		s32		temp_TeamIndex;			// 队伍房间索引
// 		u8		temp_MateIndex;			// 在队伍中成员位置
		u32		temp_SaveTime;			// 检查保存时间
		u32		temp_HeartTime;			// 心跳时间
		u32		temp_BCTime;			// 广播时间
		u32		temp_GameRoleTime;		// 获取角色数据时间

		s32     temp_TeamIndex;			// 队伍索引
		s8      temp_MateIndex;			// 队伍中的位置
		s32     temp_CopyIndex;			// 副本房间索引

		u64		temp_SkillPublicTime;						// 技能公共CD时间
		u64		temp_SkillColdTime[MAX_SKILL_COUNT];		// 技能冷却时间
		S_TEMP_SKILL_RUN	temp_SkillRun;					// 正在运行中的技能
		S_TEMP_BUFF_RUN		temp_BuffRun;					// 正在运行中的buff
		
		inline void reset()
		{
			memset(this, 0, sizeof(S_USER_TEMP_GAME));
			temp_SkillRun.reset();
			temp_BuffRun.reset();
			userindex = -1;
			user_connectid = -1;
			server_connectid = -1;
			server_clientid = -1;
			temp_CopyIndex = -1;
			bChangeMap = false;
		}
		inline void setTeamIndex(s32 tindex, s8 mindex)
		{
			temp_TeamIndex = tindex;
			temp_MateIndex = mindex;
		}
		inline void setCopyIndex(s32 index)
		{
			temp_CopyIndex = index;
		}
	};

	// 格子坐标
	struct S_GRID_BASE
	{
		s32 row;		// 行
		s32 col;		// 列
		inline bool operator==(S_GRID_BASE pos)
		{
			return (row == pos.row && col == pos.col);
		}

		inline void operator=(S_GRID_BASE pos)
		{
			row = pos.row;
			col = pos.col;
		}

		inline void operator=(S_GRID_BASE* pos)
		{
			row = pos->row;
			col = pos->col;
		}

		inline void init()
		{
			row = -1;
			col = -1;
		}
	};

	// 大格子坐标，每九个小格子就能构成一个大格子（以每一个小格子为中心都会有一个大格子）
	// 小格子表示玩家位置，大格子表示玩家可见的范围（只有这个范围内的其他用户数据是有意义的）
	// 坐标原点在左上角，Z轴向里的左手坐标系
	// top和bottom和直观视觉上的上下关系，但是如果带入到坐标系中，bottom的值>top的值
	struct S_RECT_BASE
	{
		s32 left;		// col上的范围
		s32 top;		// row上的范围
		s32 right;
		s32 bottom; 

		inline void reset()
		{
			memset(this, 0, sizeof(S_RECT_BASE));
		}
		inline void operator=(S_RECT_BASE rest)
		{
			memcpy(this, &rest, sizeof(S_RECT_BASE));
		}
		inline bool operator==(S_RECT_BASE rest)
		{

			return (left == rest.left && right == rest.right &&
					bottom == rest.bottom && top == rest.top);
		}
		// 判断一个坐标是否在区域内
		inline bool inEdge(S_GRID_BASE* inpos)
		{
			return (inpos->col >= this->left && inpos->col <= this->right && 
					inpos->row >= this->top && inpos->row <= this->bottom);
		}

		// 判断一个坐标是否不在区域内
		inline bool outEdge(S_GRID_BASE* inpos)
		{
			// 坐标在左上角，Z轴向里的左手坐标系
			return (inpos->col < this->left || inpos->col > this->right ||
					inpos->row < this->top || inpos->row > this->bottom);
		}

		// 求两个大块的交集，大块是指9*9的大格子的组合，会有交集
		// 即玩家更换大格子的时候，交集中的其他玩家数据是无需再重复请求的
		inline bool intersectRect(S_RECT_BASE* dst, S_RECT_BASE* src)
		{
			if (src->left >= right || src->top >= bottom ||
				src->right <= left || src->bottom <= top)
			{
				dst->left = -1;
				dst->right = -1;
				dst->top = -1;
				dst->bottom = -1;
				return false;
			}
			// 当前大块的左边界比src的左边界要大
			if (left > src->left) dst->left = left; else dst->left = src->left;
			if (right > src->right) dst->right = src->right; else dst->right = right;

			if (top > src->top) dst->top = top; else dst->top = src->top;
			if (bottom > src->bottom) dst->bottom = src->bottom; else dst->bottom = bottom;
			return true;
		}
	};

	// 世界节点
	struct S_WORLD_NODE
	{
		u8						type;			// 节点类型，比如角色、AI等
		u32						index;			// 下标索引，比如角色索引、怪物索引
		s32						layer;			// 在世界中的层
		S_WORLD_NODE*			upnode;			// 上一个节点
		S_WORLD_NODE*			downnode;		// 下一个节点

		inline void init(u8 _type, s32 _index, s32 _layer)
		{
			type = _type;
			index = _index;
			layer = _layer;
			upnode = nullptr;
			downnode = nullptr;
		}

		inline bool isLayer(u8 _type, s32 _layer)
		{
			return (type == _type && layer == _layer);
		}

		inline bool isLayer(u8 _type, s32 _layer, s32 _index)
		{
			return (_index != index && type == _type && layer == _layer);
		};

		struct S_USER_BASE* nodeToUser(const s32 _layer, const u32 _index, bool isself = false);
	};

	// 九宫格 格子坐标
	struct S_USER_BROADCAST
	{
		bool					isnodetree;
		bool					isedgetemp;

		S_GRID_BASE				grid_pos;		// 当前所处格子
		S_GRID_BASE				grid_big;		// 当前所处大格子

		S_RECT_BASE				edge;			// 区域
		S_RECT_BASE				edgetemp;		// 相交区域
		S_RECT_BASE				edgeold;		// 上一次的区域
	};

	struct S_USER_ATTACK
	{
		S_SPRITE_ATTACK equip;		// 装备属性值
		S_SPRITE_ATTACK total;		// 各种属性得和
	};

	// 游戏服务器上的用户数据
	struct S_USER_BASE
	{
		this_constructor(S_USER_BASE);

		// 1. 持久化数据
		S_USER_MEMBER_DATA		mem;			// 账号信息
		S_USER_ROLE				role;			// 角色详细数据
		// 2. 游戏广播数据
		S_WORLD_NODE			node;
		S_USER_BROADCAST		bc;				// 广播
		// 3.临时数据
		S_USER_ATTACK			atk;			// 角色攻击属性
		S_USER_TEMP_GAME		tmp;			// 临时数据

		inline void reset()
		{
			memset(this, 0, sizeof(S_USER_BASE));
			node.init(0, -1, -1);
		}

		s32 initNewUser();						// 初始化新手
		s32 initBornPos(const u32 mapid);		// 初始化坐标点
		s32 initReBornPos(const u32 mapid);		// 重生坐标

		void enterWorld();
		void leaveWorld();
		int isMove(S_VECTOR3* pos);				// 是否可以移动
		void moveWorld(S_VECTOR3* pos, bool& isupdatarect);		// 世界内移动

		void update();
		bool isLive();
		bool isAtkRole(S_USER_BASE* enemy);		// 是否能攻击敌人
		void roleState_SetDead();

		// 触发大格子范围内的怪物思考
		void robotThink();

		S_ROLE_STAND_SKILL_BASE* skill_IsSafe(const u8 index);
		bool skill_IsPublicTime();
		bool skill_IsColdTime(const u8 index, const u32 cdtime);
		
		s32	skill_start(void* data, void* sk);
		void skill_end(S_TEMP_SKILL_RUN_BASE* run);
		void skill_running(const s32 value);
		
		// 攻击
		void atk_ComputeInitNew();
		void atk_ComputeEquip();
		void atk_ComputeTotal();
		void atk_UpdateExp(const s32 exp);
		void updateAtk(bool isUpdateHpMp);

		//背包操作
		void  updateBagScript();//更新背包脚本
		void  updateCombatScript();//更新战斗装备脚本
		s32  bag_AddProp(S_ROLE_PROP* prop, u32 maxcount, bool& iscount);//向背包添加道具
		S_ROLE_PROP* bag_FindEmpty(u8& index);//查找一个空闲位置
		u32  bag_GetPropCount(const u32 propid);//获取道具数量
		void bag_DeleteProp(const u32 propid, const u8 count);//删除道具

		// 切图
		void changeMap(void* d, u32 size, s32 connectid, u16 cmd, u32 kind);
		//离开副本
		void leaveCopy(bool issend); 
	};
#pragma pack(pop, packing)

	inline void sendErrInfo(net::ITCPServer* ts, s32 connectid, u16 cmd, u16 childcmd, void* data, u16 size)
	{
		ts->begin(connectid, cmd);
		ts->sss(connectid, childcmd);
		ts->sss(connectid, data, size);
		ts->end(connectid);
	}
}


#endif // __WORLDDATA_H

