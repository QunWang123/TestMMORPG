#ifndef __WORLDDATA_H
#define __WORLDDATA_H
#include "INetBase.h"
#include "UserData.h"

#define C_WORLDMAP_ONE_GRID	100		// 100cmһ������
#define C_WORLDMAP_NODE_X	10		// һ�������10m��������Ը���ʵ�����������Ļ����ȫ���Ե���
#define C_WORLDMAP_NODE_Y	10		// 
#define USER_SKILL_PUBLICTIME 300
#define C_NEW_ROLE_PROTECT_LEVEL 1
#define MAX_SPRITE_COUNT	20000	// �����Ŀ���������ˣ����ֳ������ĺͣ���
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
		E_SPRITE_STATE_FREE = 0x00,		// δ֪�ͷŵ�
		E_SPRITE_STATE_LIVING,			// ����
		E_SPRITE_STATE_BUFFING,			// BUFFER��
		E_SPRITE_STATE_PICKING,			// �ɼ���	
		E_SPRITE_STATE_DEAD,			// ����
		E_SPRITE_STATE_OUTWORLD			// ����������
	};

	enum E_SKILL_USER_TYPE
	{
		E_SKILL_USE_TYPE_FREE = 0x00,	
		E_SKILL_USE_TYPE_SELF,			// ����ΪĿ�꣬360�ȼ���
		E_SKILL_USE_TYPE_DIR,			// ������
		E_SKILL_USE_TYPE_TARGET,		// ��Ŀ���360�ȵļ���	
		E_SKILL_USE_TYPE_HALF,			// ��Բ����
		E_SKILL_USE_TYPE_BUFF			// ֱ������buff�ļ���
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
		ECK_LEAVE_NO = 0x00, //�л���ͼ ������
		ECK_LEAVE_YES = 0x01,//�л���ͼ����
		ECK_SUCCESS = 0x02    //�л���ͼ �ɹ�
	};

	enum E_MAP_TYPE
	{
		EMT_PUBLIC_MAP = 0x00,  //������ͼ
		EMT_COPY_ONE = 0x01,    //���˸���
		EMT_COPY_MORE = 0x02    //���˸���
	};
#pragma pack(push, packing)
#pragma pack(1)
	// ��ʱ����

	// ����
	struct S_TEMP_SKILL_RUN_BASE
	{
		this_constructor(S_TEMP_SKILL_RUN_BASE);

		// u8		index;
		s32		skill_id;		// ����ID
		u8		skill_index;	// ��ǰ���м��ܵ��±�
		u8		skill_level;	// ���ܵȼ�
		s32		continued;		// ���ܳ���ʱ��
		s32		runingtime;		// ����ʱ��
		s32		flytime;		// ���ܷ���ʱ��
		s32		damagecount;	// �˺�������1��һ�� 

		S_VECTOR3	targetpos;	// Ŀ���
		s32		lock_index;		// �������
		u8		lock_type;		// ��������

		s32		continued_waittime;	// �ȴ����
		s32		temp_continuedtime;	// ��¼��������ʱ��
		S_VECTOR3	temp_calpos;	// ��¼���/������ǰ���ڵ�λ��

		inline void reset()
		{
			memset(this, 0, sizeof(S_TEMP_SKILL_RUN_BASE));
		}
	};

	// ���������еļ���
	struct S_TEMP_SKILL_RUN
	{
		this_constructor(S_TEMP_SKILL_RUN);
		u64			runtime;	// ��������ʱ��
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

	// ���������е�BUFF
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
			// 0000 0010	���

			// 0000 0010	state
			// 0000 0001	value		1111 1110 ȡ��
			// 0000 0011	���
			// 0000 0000	��λ��ȡ��	����

			state = state & (~value);
			// state = state ^ value;
		}
	};

	struct S_USER_TEMP_GAME
	{
		bool    bChangeMap;
		s32		userindex;				// DB�����������
		s32		user_connectid;			// ����������ϵ�id
		s32		server_connectid;		// ������ӵ����ط�������ID
		s32		server_clientid;		

// 		u8		temp_GamePage;			// ��ǰ������ڽ���
// 		s32		temp_CopyIndex;			// ������������
// 		s32		temp_TeamIndex;			// ���鷿������
// 		u8		temp_MateIndex;			// �ڶ����г�Աλ��
		u32		temp_SaveTime;			// ��鱣��ʱ��
		u32		temp_HeartTime;			// ����ʱ��
		u32		temp_BCTime;			// �㲥ʱ��
		u32		temp_GameRoleTime;		// ��ȡ��ɫ����ʱ��

		s32     temp_TeamIndex;			// ��������
		s8      temp_MateIndex;			// �����е�λ��
		s32     temp_CopyIndex;			// ������������

		u64		temp_SkillPublicTime;						// ���ܹ���CDʱ��
		u64		temp_SkillColdTime[MAX_SKILL_COUNT];		// ������ȴʱ��
		S_TEMP_SKILL_RUN	temp_SkillRun;					// ���������еļ���
		S_TEMP_BUFF_RUN		temp_BuffRun;					// ���������е�buff
		
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

	// ��������
	struct S_GRID_BASE
	{
		s32 row;		// ��
		s32 col;		// ��
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

	// ��������꣬ÿ�Ÿ�С���Ӿ��ܹ���һ������ӣ���ÿһ��С����Ϊ���Ķ�����һ������ӣ�
	// С���ӱ�ʾ���λ�ã�����ӱ�ʾ��ҿɼ��ķ�Χ��ֻ�������Χ�ڵ������û�������������ģ�
	// ����ԭ�������Ͻǣ�Z���������������ϵ
	// top��bottom��ֱ���Ӿ��ϵ����¹�ϵ������������뵽����ϵ�У�bottom��ֵ>top��ֵ
	struct S_RECT_BASE
	{
		s32 left;		// col�ϵķ�Χ
		s32 top;		// row�ϵķ�Χ
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
		// �ж�һ�������Ƿ���������
		inline bool inEdge(S_GRID_BASE* inpos)
		{
			return (inpos->col >= this->left && inpos->col <= this->right && 
					inpos->row >= this->top && inpos->row <= this->bottom);
		}

		// �ж�һ�������Ƿ���������
		inline bool outEdge(S_GRID_BASE* inpos)
		{
			// ���������Ͻǣ�Z���������������ϵ
			return (inpos->col < this->left || inpos->col > this->right ||
					inpos->row < this->top || inpos->row > this->bottom);
		}

		// ���������Ľ����������ָ9*9�Ĵ���ӵ���ϣ����н���
		// ����Ҹ�������ӵ�ʱ�򣬽����е���������������������ظ������
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
			// ��ǰ������߽��src����߽�Ҫ��
			if (left > src->left) dst->left = left; else dst->left = src->left;
			if (right > src->right) dst->right = src->right; else dst->right = right;

			if (top > src->top) dst->top = top; else dst->top = src->top;
			if (bottom > src->bottom) dst->bottom = src->bottom; else dst->bottom = bottom;
			return true;
		}
	};

	// ����ڵ�
	struct S_WORLD_NODE
	{
		u8						type;			// �ڵ����ͣ������ɫ��AI��
		u32						index;			// �±������������ɫ��������������
		s32						layer;			// �������еĲ�
		S_WORLD_NODE*			upnode;			// ��һ���ڵ�
		S_WORLD_NODE*			downnode;		// ��һ���ڵ�

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

	// �Ź��� ��������
	struct S_USER_BROADCAST
	{
		bool					isnodetree;
		bool					isedgetemp;

		S_GRID_BASE				grid_pos;		// ��ǰ��������
		S_GRID_BASE				grid_big;		// ��ǰ���������

		S_RECT_BASE				edge;			// ����
		S_RECT_BASE				edgetemp;		// �ཻ����
		S_RECT_BASE				edgeold;		// ��һ�ε�����
	};

	struct S_USER_ATTACK
	{
		S_SPRITE_ATTACK equip;		// װ������ֵ
		S_SPRITE_ATTACK total;		// �������Եú�
	};

	// ��Ϸ�������ϵ��û�����
	struct S_USER_BASE
	{
		this_constructor(S_USER_BASE);

		// 1. �־û�����
		S_USER_MEMBER_DATA		mem;			// �˺���Ϣ
		S_USER_ROLE				role;			// ��ɫ��ϸ����
		// 2. ��Ϸ�㲥����
		S_WORLD_NODE			node;
		S_USER_BROADCAST		bc;				// �㲥
		// 3.��ʱ����
		S_USER_ATTACK			atk;			// ��ɫ��������
		S_USER_TEMP_GAME		tmp;			// ��ʱ����

		inline void reset()
		{
			memset(this, 0, sizeof(S_USER_BASE));
			node.init(0, -1, -1);
		}

		s32 initNewUser();						// ��ʼ������
		s32 initBornPos(const u32 mapid);		// ��ʼ�������
		s32 initReBornPos(const u32 mapid);		// ��������

		void enterWorld();
		void leaveWorld();
		int isMove(S_VECTOR3* pos);				// �Ƿ�����ƶ�
		void moveWorld(S_VECTOR3* pos, bool& isupdatarect);		// �������ƶ�

		void update();
		bool isLive();
		bool isAtkRole(S_USER_BASE* enemy);		// �Ƿ��ܹ�������
		void roleState_SetDead();

		// ��������ӷ�Χ�ڵĹ���˼��
		void robotThink();

		S_ROLE_STAND_SKILL_BASE* skill_IsSafe(const u8 index);
		bool skill_IsPublicTime();
		bool skill_IsColdTime(const u8 index, const u32 cdtime);
		
		s32	skill_start(void* data, void* sk);
		void skill_end(S_TEMP_SKILL_RUN_BASE* run);
		void skill_running(const s32 value);
		
		// ����
		void atk_ComputeInitNew();
		void atk_ComputeEquip();
		void atk_ComputeTotal();
		void atk_UpdateExp(const s32 exp);
		void updateAtk(bool isUpdateHpMp);

		//��������
		void  updateBagScript();//���±����ű�
		void  updateCombatScript();//����ս��װ���ű�
		s32  bag_AddProp(S_ROLE_PROP* prop, u32 maxcount, bool& iscount);//�򱳰���ӵ���
		S_ROLE_PROP* bag_FindEmpty(u8& index);//����һ������λ��
		u32  bag_GetPropCount(const u32 propid);//��ȡ��������
		void bag_DeleteProp(const u32 propid, const u8 count);//ɾ������

		// ��ͼ
		void changeMap(void* d, u32 size, s32 connectid, u16 cmd, u32 kind);
		//�뿪����
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

