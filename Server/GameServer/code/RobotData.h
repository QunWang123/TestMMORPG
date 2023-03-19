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
		AI_STATE_UPDATE,	// 1.״̬����
		AI_MOVE_RAND,		// 2.����ƶ���Ϣ
		AI_MOVE_ATK,		// 3.���򹥻���Ŀ��
		AI_MOVE_RETURN,		// 6.���س�����
		AI_RELIVE_SET,		// 10.���ʼ
		AI_RELIVE_DO,		// 11.�������
		AI_CLEAR			// ����AI
	};

	enum E_ROBOT_AI_PATH_TYPE
	{
		AI_PATH_FREE = 0x00,
		AI_PATH_RIGHT,
		AI_PATH_LEFT
	};


	// �㲥����
	struct S_ROBOT_BOARDCAST
	{
		this_constructor(S_ROBOT_BOARDCAST);
		
		bool isinWorld;			// �Ƿ���������
		S_RECT_BASE	edge;		// ��������
		S_GRID_BASE	grid_pos;	// ��ǰ����С����
		S_GRID_BASE grid_big;	// ��ǰ���ڴ����

		inline void reset()
		{
			memset(this, 0, sizeof(S_ROBOT_BOARDCAST));
		}
	};

	// ��ʱ����
	struct S_ROBOT_TEMP
	{
		this_constructor(S_ROBOT_TEMP);
		u64					temp_RunningTime;		// ��¼������ʱ��
		u64					temp_SkillPublicTime;
		u64					temp_SkillColdTime[MAX_SKILL_COUNT];
		S_TEMP_SKILL_RUN	temp_SkillRun;
		S_TEMP_BUFF_RUN		temp_buffRun;
		script::SCRIPT_MONSTER_BASE* script;		// ����ű�

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
		bool			isreborn;		// �Ƿ���Ը���
		S_RECT_BASE		move_edge;		// �ƶ�����
		S_GRID_BASE		born_pos;		// ��������
		// S_GRID_BASE		cur_pos;		// ��ǰ���ڸ���

		inline void reset()
		{
			memset(this, 0, sizeof(S_ROBOT_STATUS));
		}
	};

	// �����ƶ� ս�� Ѳ��ʱ������
	struct S_ROBOT_AI_LIMIT
	{
		this_constructor(S_ROBOT_AI_LIMIT);

		u32		move_random_min;
		u32		move_random_max;
		u32		move_atk_min;			// ���򹥻�Ŀ����Сʱ��
		u32		move_atk_max;			// ���򹥻�Ŀ�����ʱ��

		u64		time_think;				// ˼�����
		u32		time_move;				// ��ǰ�ƶ����
		u32		time_move_atk;			// ����Ŀ��ļ��
		u32     time_tempreturn;
		u32		time_relive;			// ������
		u32		time_movereturn;		// ���ؼ��
		u32		time_renew;				// ״̬�ָ����
		u32		time_renewcount;		// ״̬�ָ��ۼƴ���

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

		s32				skill_index;		// ��ǰʹ�õļ�������
		u8				skill_level;		// ��ǰʹ�õļ��ܵȼ�
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
		E_ROBOT_AI_THINK_TYPE	think;		// ˼������
		S_ROBOT_AI_LIMIT		limit;		// ������Ϣ
		S_ROBOT_AI_BASE			base;		// ������Ϣ	
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

	// ��������
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
		// 1.��������Ϸ����
		S_ROBOT_BASE		data;
		// 2.�����˹㲥��������
		S_WORLD_NODE		node;		// �ڵ���Ϣ
		S_ROBOT_BOARDCAST	bc;			// �㲥��Ϣ

		// 3.��ʱ
		S_ROBOT_TEMP		tmp;

		// ����
		bool isT();
		bool isLive();
		u8	 getSpriteCollideType();
		void setLock(u8 kind, u32 index);
		bool isLockDistance(const u8 type, const u32 index);
		void resetDead();
		void setDead();
		void setMoveReturn();
		
		// ������
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

