#ifndef ___USERDATA_H
#define ___USERDATA_H

#include "IDefine.h"
#include <string>

#define USER_MAX_MEMBER_CHECK	21
#define USER_MAX_MEMBER			20
#define USER_MAX_PASS			20
#define USER_MAX_KEY			11
#define USER_MAX_NICK			20
#define USER_MAX_ROLE			3

//�������� һ��Ҫ������������������Ϊ������Ʋ�Ĺ�ϵ
#define MAX_LAYER_COUNT			2000
#define MIN_LAYER_COUNT			1 //��=0 �ǹ�����ͼ ��������

#define USER_BORN_MAPID			1		// ��ҳ�����ͼID
#define MAX_ROLE_LEVEL			1000
#define MAX_SKILL_COUNT			10
#define MAX_SKILL_PUBLICTIME	300
#define MAX_BUFF_COUNT			10
#define MAX_ROOM_COUNT			100
#define MAX_MAP_ID				110
#define MAX_LINE_COUNT			10

#define USER_MAX_BAG			150 //��󱳰���������
#define USER_INIT_BAG			64 //��ʼ�ֿ��������
#define USER_MAX_WAREHOUSE		150 //���ֿ��������
#define USER_INIT_WAREHOUSE		64 //��ʼ�ֿ��������
#define USER_MAX_EQUIP			15  //������װ������
#define MAX_GATE_COUNT			20//�����������

#define USER_GETBAG_TIME		30
#define USER_GETCOMPAT_TIME		3
#define USER_GETCHANGEMAP_TIME  3
#define  USER_COPY_TIME  5

#define C_ROLE_INNATE_LEN		30
#define C_ROLE_STATUS_LEN		19
#define C_ROLE_EXP_LEN			2


#define  C_TEAM_MAX           200//��ǰ����������
#define  C_TEAM_PLAYERMAX     4//�����������
#define  C_TEAM_START_POS     1 //���鿪ʼλ��

#define  USER_MAX_GOLD           100000000  //���˽�����1e
#define  USER_MAX_WAREHOUSE_GOLD 1000000000 //�ֿ������10e

namespace app
{

	enum E_MEMBER_STATE
	{
		M_FREE = 0x00,			// ʹ��-����
		M_LIMIT = 0x01,			// ����
		M_STOP = 0x02,			// ���
		M_SELECTROLE = 0x03,	// ѡ���ɫ
		M_LOGINING = 0x04,		// ��½��
		M_LOGINEND = 0x05,		// ʹ��-��¼
		M_SAVING = 0x06,		// ������

	};

	enum E_GAMESTATE
	{
		GAME_FREE = 0x00,
		GAME_WAIT,
		GAME_RUN,
		GAME_ATK,
		GAME_DIED,
	};

	//��������
	enum E_PROP_TYPE
	{
		EPT_UNKNOWN = 0x00,
		EPT_EQUIP = 0x01, //װ��
		EPT_CONSUME = 0x02,//������ĵ���
		EPT_GOLD = 0x08  //���
	};
	//������Դ
	enum E_PROP_FROM
	{
		EPF_UNKNOWN = 0x00,
		EPF_DATASERVER = 0x01, //���ݷ�����
		EPF_GAMESERVER = 0x02, //��Ϸ������
		EPF_PLAYERFORGE = 0x03, //��Ҵ���
		EPF_ACTIVE = 0x04,     //�
		EPF_MONSTER = 0x05,    //�������
		EPF_BOSS = 0x06,       //BOSS����
		EPF_NPC = 0x07,         //NPC
		EPF_TASK = 0x08         //������
	};
	//װ�����������
	enum E_PROP_EQUIP
	{
		EPE_UNKNOWN = 0x00,
		EPE_WEQPON = 0x01,		//����
		EPE_HEAD = 0x02,		//ͷ��
		EPE_CLOTH = 0x03,		//�·�
		EPE_DECORATIVE = 0x04,	// װ��
		EPE_SHOES = 0x05, //Ь��
		EPE_NECKLACE = 0x06,//����
		EPE_RING = 0x07,//��ָ 
		EPE_BANGLE = 0x08,//����
	};
	
	//ս��װ��λ��
	enum E_ROLE_EQUIPOS
	{
		ERP_WEQPON = 0x00,		// ����
		ERP_HEAD = 0x01,		// ͷ��
		ERP_CLOTH = 0x02,		// �·�
		ERP_DECORATIVE = 0x03,		// װ�Σ����֮��ģ�
		ERP_SHOES = 0x04,		// Ь��
		ERP_NECKLACE = 0x05,		// ����
		ERP_RING_LEFT = 0x06,		// ��ָ��
		ERP_RING_RIGHT = 0x07,		// ��ָ��
		ERP_BANGLE_LEFT = 0x08,		// ������
		ERP_BANGLE_RIGHT = 0x09,		// ������
		ERP_COUNT = 0xA  //��ǰװ��λ����
	};

	//���������
	enum E_PROP_CONSUME
	{
		EPC_UNKNOWN = 0x00,
		EPC_HP = 0x01,  //��������
		EPC_MP = 0x02,  //����ħ��ֵ
		EPC_EXP = 0x03, //���Ӿ���
		EPC_GEM = 100
	};
	//���������--��ʯ����
	enum E_PROP_CONSUME_GEM
	{
		EPC_GEM_PURPLE = 0x01, //�ϱ�ʯ
		EPC_GEM_BLUE = 0x02,   //����ʯ
		EPC_GEM_YELLOW = 0x03, //�Ʊ�ʯ
		EPC_GEM_GREEN = 0x04,  //�̱�ʯ
		EPC_GEM_RED = 0x05     //�챦ʯ
	};

	//����뿪����ԭ��
	enum E_TEAMLEAVE_TYPE
	{
		EDT_SELF = 0x00,//����Լ��뿪
		EDT_DISCONNET_USER, //��ҵ���
		EDT_DISCONNET_GATE, //���غ�TEAM�Ͽ�
		EDT_DISCONNET_GAME, //��Ϸ����TEAM�Ͽ�
		EDT_HEART, //������ҵ���
		EDT_COPY  //�������뿪
	};

#pragma pack(push, packing)
#pragma pack(1)

	struct S_VECTOR3
	{
		this_constructor(S_VECTOR3);
		int x;
		int y;
		int z;
		inline void operator=(S_VECTOR3& pos)
		{
			x = pos.x;
			y = pos.y;
			z = pos.z;
		}
	};

	// ��Ϸ�˺ţ���ɫ�������ݣ���Ҫ����ѡ���ɫʹ��
	struct S_USER_MEMBER_ROLE
	{
		u64			id;						// ��ɫID
		u8			job;					// ְҵ
		u8			sex;
		u32			clothid;
		u32			weaponid;
		u16			level;
		char		nick[USER_MAX_NICK];

		inline bool isT() const { return id > 0; };
		inline bool isT(const u64 value) { return (id == value); };
		inline void reset() { memset(this, 0, sizeof(S_USER_MEMBER_ROLE)); };

	};

	struct S_USER_MEMBER_DATA
	{
		u64			id;						// �˺�Ψһ���
		u8			state;					// �˺�״̬
		u16			areaID;					// ����ID
		u8			tableID;				// ���ڱ�ID�������Ż����ݿ��
		char		name[USER_MAX_MEMBER];	// �˺�

		u32			timeCreate;				// ����ʱ��
		u32			timeLastLogin;			// ��һ�ε�¼ʱ��

		inline bool isT() const { return id > 0; };
		inline bool isT_ID(const u64 value) { return (id == value); };
		inline bool isT_Name(const char* value) { return (strcmp(name, value) == 0); };
	};

	struct S_USER_MEMBER_TEMP
	{
		u8			roleindex;				// ��¼��ǰ����ʹ�õĽ�ɫ����
		u32			userindex;				// ��¼��ǰ����ʹ�õĽ�ɫ��¼����
		u32			temp_HeartTime;			// ����ʱ��
	};

	struct S_USER_MEMBER_BASE
	{
		this_constructor(S_USER_MEMBER_BASE);

		S_USER_MEMBER_ROLE			role[USER_MAX_ROLE];	// ��Ϸ�˺ţ��������
		S_USER_MEMBER_DATA			mem;	// ��ɫ��������
		S_USER_MEMBER_TEMP			tmp;	// ��ʱ��¼����

		inline s32 findFreeRoleIndex()
		{
			for (int i = 0; i < USER_MAX_ROLE; i++)
			{
				if (role[i].id == 0) return i;
			}
			return -1;
		}
	};
	// �����Ϸ����

	// ��ɫ����

	// ************************************************************
	// 1. ��ɫ��������
	// ************************************************************
	// ��ɫ��������
	struct S_ROLE_BASE_INNATE
	{
		this_constructor(S_ROLE_BASE_INNATE);
		u64							id;		// ��ɫid;
		u8							job;	// ְҵ
		u8							sex;	// �Ա�
		char						nick[USER_MAX_NICK];		// �ǳ�

		inline bool isT() const 
		{
			return id > 0;
		}
		inline bool isT_ID(const u64 value)
		{
			return (value == id);
		}
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_INNATE));
		}
	};

	struct S_ROLE_BASE_EXP
	{
		this_constructor(S_ROLE_BASE_EXP);
		u16				level;		// �ȼ�
		u32				currexp;	// ��ǰ����ֵ
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_EXP));
		}
	};

	struct S_ROLE_BASE_HONOR
	{
		this_constructor(S_ROLE_BASE_HONOR);
		u32				medalnum;	// ��������
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_HONOR));
		}
	};

	struct S_ROLE_BASE_ECON
	{
		this_constructor(S_ROLE_BASE_ECON);
		u32				gold;		// ���
		u32				diamonds;	// ��ʯ
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_ECON));
		}
	};

	struct S_ROLE_BASE_STATUS
	{
		this_constructor(S_ROLE_BASE_STATUS);
		u8				state;		
		s16				face;		// ����
		s32				mapid;		// ��ǰ���ڵ�ͼID
		S_VECTOR3		pos;		// ��ǰ����
		s32				c_mapid;	// ����ĵ�ͼID
		S_VECTOR3		c_pos;		// ��������
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_STATUS));
		}
		//���븱�������˳����� ����
		inline void copyMapid(bool isreturn)
		{
			if (isreturn)
			{
				mapid = c_mapid;
				pos = c_pos;
			}
			else
			{
				c_mapid = mapid;
				c_pos = pos;
			}
		}
	};

	struct S_ROLE_BASE_LIFE
	{
		this_constructor(S_ROLE_BASE_LIFE);
		s32				hp;			// ��ǰ����ֵ
		s32				mp;			// ��ǰħ��ֵ
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_LIFE));
		}
	};

	// 1.��ɫ��������
	struct S_ROLE_BASE
	{
		this_constructor(S_ROLE_BASE);
		S_ROLE_BASE_INNATE			innate;		// ��������
		S_ROLE_BASE_EXP				exp;		// ����
		S_ROLE_BASE_HONOR			honor;		// ����
		S_ROLE_BASE_ECON			econ;		// ����
		S_ROLE_BASE_STATUS			status;		// ״̬
		S_ROLE_BASE_LIFE			life;		// ����
		inline void reset()
		{
			innate.reset();
			exp.reset();
			honor.reset();
			econ.reset();
			status.reset();
			life.reset();
		}
	};

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************

	//����
	struct  S_ROLE_PROP_BASE
	{
		this_constructor(S_ROLE_PROP_BASE)

		s32    id;         //����ID
		u8     sourcefrom; //��Դ
		u8     type;       //����
		u16    count;      //����
		s64    createtime; //����ʱ��
		s64    uniqueid;   //Ψһid
		u32    money;      //���ۼ۸�

	};
	// װ�����ߵ�����
	struct  S_ROLE_PROP_EQUIP
	{
		this_constructor(S_ROLE_PROP_EQUIP)
		//������ɵ�����
		u16     p_atk;//������
		u16     m_atk;//ħ������
		u16     p_defend;//�������
		u16     m_defend;//ħ������
		u8      crit; //����
		u8      dodge;//����
		u16     hp;   //����
		u16		mp;
		char    nick[USER_MAX_NICK];//����װ���ߵ��ǳ�
	};

	//��������ı�ʯ
	struct  S_ROLE_PROP_GEM
	{
		this_constructor(S_ROLE_PROP_GEM)
		//��ʯ�����ӹ̶�������
		u8     gem_purple; //�ϱ�ʯ����   +������
		u8     gem_blue;   //����ʯ����   +ħ������
		u8     gem_yellow; //�Ʊ�ʯ����   +�������
		u8     gem_green;  //�̱�ʯ����   +ħ������
		u8     gem_red;    //�챦ʯ����   +����
		inline u8 GemNum()
		{
			return gem_purple + gem_blue + gem_yellow + gem_green + gem_red;
		}
	};
	struct  S_ROLE_PROP_TEMP
	{
		void*     script_prop;//�ű�ָ���ַ
	};
	struct  S_ROLE_PROP
	{
		this_constructor(S_ROLE_PROP)

		S_ROLE_PROP_BASE   base;  //��������
		S_ROLE_PROP_EQUIP  equip; //װ������
		S_ROLE_PROP_GEM    gem;   //��ʯ����
		S_ROLE_PROP_TEMP   temp;

		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_PROP));
			temp.script_prop = NULL;
		}
		inline bool IsT()
		{
			if (base.id > 0 && base.count > 0)
			{
				return true;
			}
			return false;
		}
		inline bool isSame(S_ROLE_PROP* prop)
		{
			if (this->base.id != prop->base.id || this->base.type != prop->base.type ||
				this->base.money != prop->base.money)
				return false;
			return true;
		}
		inline u32 sendSize()
		{
			u32 size = sizeof(S_ROLE_PROP_BASE) + sizeof(S_ROLE_PROP_EQUIP) + sizeof(S_ROLE_PROP_GEM);

			return size;
		}
		// ��֤������Ч��(˳��ḳֵ�ű���ַ)
		bool isPropValid();
	};

	//��ɫս��װ�� ����������崩����װ��
	struct S_ROLE_STAND_COMBAT
	{
		this_constructor(S_ROLE_STAND_COMBAT)
		S_ROLE_PROP   equip[USER_MAX_EQUIP];
		inline void Reset() { memset(this, 0, sizeof(S_ROLE_STAND_COMBAT)); }
	};

	// ����
	struct S_ROLE_STAND_SKILL_BASE
	{

		u32							id;
		u8							level;
		u8							quick_index;		// ������±�
		inline bool isT()
		{
			if (id > 0 && level > 0) return true;
			return false;
		}
	};

	struct S_ROLE_STAND_SKILL
	{
		this_constructor(S_ROLE_STAND_SKILL);
		S_ROLE_STAND_SKILL_BASE		skill[MAX_SKILL_COUNT];				//ӵ�еļ���
	};

	// ����
	struct S_ROLE_STAND_BAG
	{
		this_constructor(S_ROLE_STAND_BAG)

		u8   num;		// ��ǰ�������� ���150������64������ʵһ��ʼ�ʹ�����150���Ŀռ䣬����ֱ�Ӹ���150��
		S_ROLE_PROP    bags[USER_MAX_BAG];
		// S_ROLE_PROP_TEMP temp[USER_MAX_BAG];
		inline void Reset() { memset(this, 0, sizeof(S_ROLE_STAND_BAG)); }
		inline S_ROLE_PROP* findEmpty(u8& pos)
		{
			if (num > USER_MAX_BAG) return nullptr;
			for (u8 i = 0; i < num; i++)
			{
				if (bags[i].IsT()) continue;
				pos = i;
				return &bags[i];
			}
			return nullptr;
		}
		inline u8 getEmptyCount()
		{
			u8 count = 0;
			for (u8 i = 0; i < num; i++)
			{
				if (bags[i].IsT()) continue;
				count++;
			}
			return count;
		}
	};

	//202104 ���鹥���ṹ ������� ���� NPC ����
	struct S_SPRITE_ATTACK
	{
		this_constructor(S_SPRITE_ATTACK)
		s32   hp;
		s32   mp;
		s32   p_atk;//������
		s32   m_atk;//ħ������
		s32   p_defend;//�������
		s32   m_defend;//ħ������
		s32   dodge;//����
		s32   crit; //����
		s32   dechp;//�˺�����
		s32   speed;//�ƶ��ٶ�
		inline void reset()
		{
			memset(this, 0, sizeof(S_SPRITE_ATTACK));
		}
	};


	// 2. ��ɫ��������
	struct S_ROLE_STAND
	{
		this_constructor(S_ROLE_STAND);
		S_ROLE_STAND_COMBAT			combat;		// ��ɫս��װ��
		S_ROLE_STAND_BAG			bag;		// ����
		S_ROLE_STAND_SKILL			myskill;	// ����
		S_SPRITE_ATTACK				atk;		// ��һ�����������ֵ
		// S_ROLE_STAND_TASK		task;		// ����

	};

	struct S_USER_ROLE
	{
		this_constructor(S_USER_ROLE);

		S_ROLE_BASE					base;
		S_ROLE_STAND				stand;

		inline void reset() 
		{ 
			memset(this, 0, sizeof(S_USER_ROLE)); 
		};
	};

	// ��ʱ����
	struct S_USER_TEMP_BASE
	{
		s8			line;
		s32			userindex;			// DB��������±�����
		s32			user_connectid;		// ��������ID
		s32			server_connectid;	// �ͻ�������ID
		s32			server_clientid;	// �ͻ���ID

		u32			temp_SaveTime;		// ��鱣��ʱ��
		u32			temp_HeartTime;		// ����ʱ��
// 		s32			temp_CopyIndex;		// ������������
		s32			temp_TeamIndex;		// ���鷿������
		u8			temp_MateIndex;		// �ڶ����г�Աλ��

		inline void reset()
		{
			memset(this, 0, sizeof(S_USER_TEMP_BASE));
			line					= -1;
			user_connectid			= -1;
			server_connectid		= -1;
			server_clientid			= -1;
			userindex				= -1;
			temp_TeamIndex			= -1;
			temp_MateIndex			= -1;
		}
		inline void setTeamIndex(s32 tindex, s8 mindex)
		{
			temp_TeamIndex = tindex;
			temp_MateIndex = mindex;
		}
	};
	//*************************************************************************
//��������
//�����Ա���� �������ݶ�����ͨ��userindex���ҵĵ�
	struct S_TEAM_MATE
	{
		bool  isused;     //�Ƿ�ʹ��
		bool  isleader;   //�ǲ��Ƕӳ� 
		u8    mateindex;  //�ڶ����е�λ��
		u32   userindex;  //����ڷ������е�λ��
		u64   memid;      //���ΨһID

		inline bool isT()
		{
			return memid > 10000;
		}
		inline void reset()
		{
			memset(this, 0, sizeof(S_TEAM_MATE));
		}
	};

	//��������
	struct S_TEAM_BASE
	{
		bool  isused;//�Ƿ�ʹ��
		s32   index; //��������
		s64   createtime;//���鴴��ʱ��

		S_TEAM_MATE    mates[C_TEAM_PLAYERMAX];//ȫ����Ա

		inline void reset()
		{
			memset(this, 0, sizeof(S_TEAM_BASE));
			isused = false;
			index = -1;
		}

		u32 getNum();
		bool isLeader(s8 mateindex);
		S_TEAM_MATE* findMate(s8 mateindex);
		S_TEAM_MATE* findLeader();
#ifdef APP_CENTER_SERVER
		void setNewUserTeam(u32 userindex, s8 mateindex, bool isleader);
		S_TEAM_MATE* setNewLeader();
		S_TEAM_MATE* setNewLeader(s8 mateindex);
		S_TEAM_MATE* findEmptyMate();
#endif 
	};

	//�������������ܹ���
	struct S_SERVER_MANAGER
	{
		u8   state;//�������Ƿ�����
		u8   line; //������
		s32  id; //������ID
		s32  memory;//�ڴ�ֵ
		s32  online;//��������
		char ip[MAX_IP_LEN];
		u16  port;

		inline void reset()
		{
			memset(this, 0, sizeof(S_SERVER_MANAGER));
		}
	};
#pragma pack(pop, packing)
}
#endif