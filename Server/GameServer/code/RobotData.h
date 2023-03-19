#ifndef ____ROBOTDATA
#define ____ROBOTDATA

#include "IDefine.h"
#include "WorldData.h"
#include "WorldScript.h"

namespace app
{
	enum E_ROBOT_AI_THINK_TYPE
	{
		AI_FREE = 0x00,		// 
		AI_STATE_UPDATE,	// 1.状态更新
		AI_MOVE_RAND,		// 2.随机移动信息
		AI_MOVE_ATK,		// 3.移向攻击个目标
		AI_MOVE_RETURN,		// 6.返回出生点
		AI_RELIVE_SET,		// 10.复活开始
		AI_RELIVE_DO,		// 11.复活结束
		AI_CLEAR			// 结束AI
	};

	enum E_ROBOT_AI_PATH_TYPE
	{
		AI_PATH_FREE = 0x00,
		AI_PATH_RIGHT,
		AI_PATH_LEFT
	};


	// 广播数据
	struct S_ROBOT_BOARDCAST
	{
		this_constructor(S_ROBOT_BOARDCAST);
		
		bool isinWorld;			// 是否在世界中
		S_RECT_BASE	edge;		// 更新区域
		S_GRID_BASE	grid_pos;	// 当前所在小格子
		S_GRID_BASE grid_big;	// 当前所在大格子

		inline void reset()
		{
			memset(this, 0, sizeof(S_ROBOT_BOARDCAST));
		}
	};

	// 临时数据
	struct S_ROBOT_TEMP
	{
		this_constructor(S_ROBOT_TEMP);
		u64					temp_RunningTime;		// 记录下运行时间
		u64					temp_SkillPublicTime;
		u64					temp_SkillColdTime[MAX_SKILL_COUNT];
		S_TEMP_SKILL_RUN	temp_SkillRun;
		S_TEMP_BUFF_RUN		temp_buffRun;
		script::SCRIPT_MONSTER_BASE* script;		// 怪物脚本

		inline void reset()
		{
			int len = (MAX_SKILL_COUNT + 2) * 8;
			memset(this, 0, len);
			temp_SkillRun.reset();
			temp_buffRun.reset();
			script = NULL;
		}
	};

	struct S_ROBOT_STATUS
	{
		this_constructor(S_ROBOT_STATUS);

		s32				id;
		u8				dir;
		// s32				scriptindex;
		u8				state;
		u32				mapid;
		s32				hp;
		s32				mp;
		bool			isreborn;		// 是否可以复活
		S_RECT_BASE		move_edge;		// 移动区域
		S_GRID_BASE		born_pos;		// 出生格子
		// S_GRID_BASE		cur_pos;		// 当前所在格子

		inline void reset()
		{
			memset(this, 0, sizeof(S_ROBOT_STATUS));
		}
	};

	// 怪物移动 战斗 巡逻时间限制
	struct S_ROBOT_AI_LIMIT
	{
		this_constructor(S_ROBOT_AI_LIMIT);

		u32		move_random_min;
		u32		move_random_max;
		u32		move_atk_min;			// 移向攻击目标最小时间
		u32		move_atk_max;			// 移向攻击目标最大时间

		u64		time_think;				// 思考间隔
		u32		time_move;				// 当前移动间隔
		u32		time_move_atk;			// 移向目标的间隔
		u32     time_tempreturn;
		u32		time_relive;			// 复活间隔
		u32		time_movereturn;		// 返回间隔
		u32		time_renew;				// 状态恢复间隔
		u32		time_renewcount;		// 状态恢复累计次数

		inline void reset()
		{
			memset(this, 0, sizeof(S_ROBOT_AI_LIMIT));
		}
	};

	struct S_ROBOT_AI_BASE
	{
		this_constructor(S_ROBOT_AI_BASE);

		u8				lock_type;
		s32				lock_id;
		u8				lock_sub_type;
		s32				lock_sub_id;

		s32				skill_index;		// 当前使用的技能索引
		u8				skill_level;		// 当前使用的技能等级
		u8				pathtype;
		bool			pathsave;
		S_GRID_BASE		path_coord1;
		S_GRID_BASE		path_coord2;

		inline void reset()
		{
			memset(this, 0, sizeof(S_ROBOT_AI_BASE));
			path_coord1.init();
			path_coord2.init();
			lock_id = -1;
		}
	};


	struct S_ROBOT_AI
	{
		this_constructor(S_ROBOT_AI);
		E_ROBOT_AI_THINK_TYPE	think;		// 思考类型
		S_ROBOT_AI_LIMIT		limit;		// 限制信息
		S_ROBOT_AI_BASE			base;		// 基础信息	
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROBOT_AI));
		}
		inline void setThink(E_ROBOT_AI_THINK_TYPE value) { think = value; };
		void randTime(u32 id);
		bool isTimeMoveRND();
		bool isTimeMoveATK();
		bool isTimeRenew();
		bool isTimeMoveReturn();
	
	};

	// 基础数据
	struct S_ROBOT_BASE
	{
		this_constructor(S_ROBOT_BASE);
		S_ROBOT_STATUS		status;
		S_ROBOT_AI			ai;
		S_SPRITE_ATTACK		atk;
	};

	struct S_ROBOT
	{
		this_constructor(S_ROBOT);
		// 1.机器人游戏数据
		S_ROBOT_BASE		data;
		// 2.机器人广播查找数据
		S_WORLD_NODE		node;		// 节点信息
		S_ROBOT_BOARDCAST	bc;			// 广播信息

		// 3.临时
		S_ROBOT_TEMP		tmp;

		// 公用
		bool isT();
		bool isLive();
		u8	 getSpriteCollideType();
		void setLock(u8 kind, u32 index);
		bool isLockDistance(const u8 type, const u32 index);
		void resetDead();
		void setDead();
		void setMoveReturn();
		
		// 世界内
		void init(const u8 kind, const u32 index);
		bool createInWorld(const s32 mapid, const s32 row, const s32 col);
		bool enterWorld(const s32 mapid, S_GRID_BASE* pos);
		bool leaveWorld();
		bool findWorldCoordinate(const s32 mapid, S_GRID_BASE* curpos, S_GRID_BASE* newpos);
		bool moveWorld(u32 row, u32 col, u8 dir);

		// event
		void event_AI(const s32 index);
		bool event_Think(const s32 index);
		void event_SetLock(const u8 kind, const u32 index);
		void event_RoleAtk(const s32 index);

		// skill
		bool skill_start();
		void skill_running(const s32 value);
		void skill_end(S_TEMP_SKILL_RUN_BASE* run);
		bool skill_IsPublicTime();
		bool skill_IsColdTime(const u8 skill_index, const u32 time);

		// think
		void Think_StateUpdate();
		void Think_MoveRand();
		void Think_SetReborn();
		void Think_Reborn();
		void Think_MoveAtk();
		void Think_Move(u8 kind);
		void Think_MoveReturn();
	};

}

#endif

