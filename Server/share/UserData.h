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

//最大层数量 一定要大于最大玩家数量，以为副本设计层的关系
#define MAX_LAYER_COUNT			2000
#define MIN_LAYER_COUNT			1 //层=0 是公共地图 其他副本

#define USER_BORN_MAPID			1		// 玩家出生地图ID
#define MAX_ROLE_LEVEL			1000
#define MAX_SKILL_COUNT			10
#define MAX_SKILL_PUBLICTIME	300
#define MAX_BUFF_COUNT			10
#define MAX_ROOM_COUNT			100
#define MAX_MAP_ID				110
#define MAX_LINE_COUNT			10

#define USER_MAX_BAG			150 //最大背包格子数量
#define USER_INIT_BAG			64 //初始仓库格子数量
#define USER_MAX_WAREHOUSE		150 //最大仓库格子数量
#define USER_INIT_WAREHOUSE		64 //初始仓库格子数量
#define USER_MAX_EQUIP			15  //玩家最大装备数量
#define MAX_GATE_COUNT			20//最大网关数量

#define USER_GETBAG_TIME		30
#define USER_GETCOMPAT_TIME		3
#define USER_GETCHANGEMAP_TIME  3
#define  USER_COPY_TIME  5

#define C_ROLE_INNATE_LEN		30
#define C_ROLE_STATUS_LEN		19
#define C_ROLE_EXP_LEN			2


#define  C_TEAM_MAX           200//当前世界队伍个数
#define  C_TEAM_PLAYERMAX     4//队伍最大人数
#define  C_TEAM_START_POS     1 //队伍开始位置

#define  USER_MAX_GOLD           100000000  //个人金币最大1e
#define  USER_MAX_WAREHOUSE_GOLD 1000000000 //仓库金币最大10e

namespace app
{

	enum E_MEMBER_STATE
	{
		M_FREE = 0x00,			// 使用-空闲
		M_LIMIT = 0x01,			// 禁用
		M_STOP = 0x02,			// 封号
		M_SELECTROLE = 0x03,	// 选择角色
		M_LOGINING = 0x04,		// 登陆中
		M_LOGINEND = 0x05,		// 使用-登录
		M_SAVING = 0x06,		// 保存中

	};

	enum E_GAMESTATE
	{
		GAME_FREE = 0x00,
		GAME_WAIT,
		GAME_RUN,
		GAME_ATK,
		GAME_DIED,
	};

	//道具类型
	enum E_PROP_TYPE
	{
		EPT_UNKNOWN = 0x00,
		EPT_EQUIP = 0x01, //装备
		EPT_CONSUME = 0x02,//玩家消耗道具
		EPT_GOLD = 0x08  //金币
	};
	//道具来源
	enum E_PROP_FROM
	{
		EPF_UNKNOWN = 0x00,
		EPF_DATASERVER = 0x01, //数据服务器
		EPF_GAMESERVER = 0x02, //游戏服务器
		EPF_PLAYERFORGE = 0x03, //玩家打造
		EPF_ACTIVE = 0x04,     //活动
		EPF_MONSTER = 0x05,    //怪物掉落
		EPF_BOSS = 0x06,       //BOSS掉落
		EPF_NPC = 0x07,         //NPC
		EPF_TASK = 0x08         //任务奖励
	};
	//装备类道具类型
	enum E_PROP_EQUIP
	{
		EPE_UNKNOWN = 0x00,
		EPE_WEQPON = 0x01,		//武器
		EPE_HEAD = 0x02,		//头盔
		EPE_CLOTH = 0x03,		//衣服
		EPE_DECORATIVE = 0x04,	// 装饰
		EPE_SHOES = 0x05, //鞋子
		EPE_NECKLACE = 0x06,//项链
		EPE_RING = 0x07,//戒指 
		EPE_BANGLE = 0x08,//手镯
	};
	
	//战斗装备位置
	enum E_ROLE_EQUIPOS
	{
		ERP_WEQPON = 0x00,		// 武器
		ERP_HEAD = 0x01,		// 头盔
		ERP_CLOTH = 0x02,		// 衣服
		ERP_DECORATIVE = 0x03,		// 装饰（翅膀之类的）
		ERP_SHOES = 0x04,		// 鞋子
		ERP_NECKLACE = 0x05,		// 项链
		ERP_RING_LEFT = 0x06,		// 戒指左
		ERP_RING_RIGHT = 0x07,		// 戒指右
		ERP_BANGLE_LEFT = 0x08,		// 手镯左
		ERP_BANGLE_RIGHT = 0x09,		// 手镯右
		ERP_COUNT = 0xA  //当前装备位数量
	};

	//消耗类道具
	enum E_PROP_CONSUME
	{
		EPC_UNKNOWN = 0x00,
		EPC_HP = 0x01,  //增加生命
		EPC_MP = 0x02,  //增加魔法值
		EPC_EXP = 0x03, //增加经验
		EPC_GEM = 100
	};
	//消耗类道具--宝石分类
	enum E_PROP_CONSUME_GEM
	{
		EPC_GEM_PURPLE = 0x01, //紫宝石
		EPC_GEM_BLUE = 0x02,   //蓝宝石
		EPC_GEM_YELLOW = 0x03, //黄宝石
		EPC_GEM_GREEN = 0x04,  //绿宝石
		EPC_GEM_RED = 0x05     //红宝石
	};

	//玩家离开队伍原因
	enum E_TEAMLEAVE_TYPE
	{
		EDT_SELF = 0x00,//玩家自己离开
		EDT_DISCONNET_USER, //玩家掉线
		EDT_DISCONNET_GATE, //网关和TEAM断开
		EDT_DISCONNET_GAME, //游戏服和TEAM断开
		EDT_HEART, //心跳玩家掉线
		EDT_COPY  //副本内离开
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

	// 游戏账号，角色基础数据，主要用于选择角色使用
	struct S_USER_MEMBER_ROLE
	{
		u64			id;						// 角色ID
		u8			job;					// 职业
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
		u64			id;						// 账号唯一标记
		u8			state;					// 账号状态
		u16			areaID;					// 区域ID
		u8			tableID;				// 所在表ID，用于优化数据库表
		char		name[USER_MAX_MEMBER];	// 账号

		u32			timeCreate;				// 创建时间
		u32			timeLastLogin;			// 上一次登录时间

		inline bool isT() const { return id > 0; };
		inline bool isT_ID(const u64 value) { return (id == value); };
		inline bool isT_Name(const char* value) { return (strcmp(name, value) == 0); };
	};

	struct S_USER_MEMBER_TEMP
	{
		u8			roleindex;				// 记录当前正在使用的角色索引
		u32			userindex;				// 记录当前正在使用的角色记录索引
		u32			temp_HeartTime;			// 心跳时间
	};

	struct S_USER_MEMBER_BASE
	{
		this_constructor(S_USER_MEMBER_BASE);

		S_USER_MEMBER_ROLE			role[USER_MAX_ROLE];	// 游戏账号，相关数据
		S_USER_MEMBER_DATA			mem;	// 角色基础数据
		S_USER_MEMBER_TEMP			tmp;	// 临时记录数据

		inline s32 findFreeRoleIndex()
		{
			for (int i = 0; i < USER_MAX_ROLE; i++)
			{
				if (role[i].id == 0) return i;
			}
			return -1;
		}
	};
	// 玩家游戏数据

	// 角色数据

	// ************************************************************
	// 1. 角色基础数据
	// ************************************************************
	// 角色先天属性
	struct S_ROLE_BASE_INNATE
	{
		this_constructor(S_ROLE_BASE_INNATE);
		u64							id;		// 角色id;
		u8							job;	// 职业
		u8							sex;	// 性别
		char						nick[USER_MAX_NICK];		// 昵称

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
		u16				level;		// 等级
		u32				currexp;	// 当前经验值
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_EXP));
		}
	};

	struct S_ROLE_BASE_HONOR
	{
		this_constructor(S_ROLE_BASE_HONOR);
		u32				medalnum;	// 奖牌数量
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_HONOR));
		}
	};

	struct S_ROLE_BASE_ECON
	{
		this_constructor(S_ROLE_BASE_ECON);
		u32				gold;		// 金币
		u32				diamonds;	// 钻石
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_ECON));
		}
	};

	struct S_ROLE_BASE_STATUS
	{
		this_constructor(S_ROLE_BASE_STATUS);
		u8				state;		
		s16				face;		// 朝向
		s32				mapid;		// 当前所在地图ID
		S_VECTOR3		pos;		// 当前坐标
		s32				c_mapid;	// 保存的地图ID
		S_VECTOR3		c_pos;		// 保存坐标
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_STATUS));
		}
		//进入副本或者退出副本 设置
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
		s32				hp;			// 当前生命值
		s32				mp;			// 当前魔法值
		inline void reset()
		{
			memset(this, 0, sizeof(S_ROLE_BASE_LIFE));
		}
	};

	// 1.角色基础数据
	struct S_ROLE_BASE
	{
		this_constructor(S_ROLE_BASE);
		S_ROLE_BASE_INNATE			innate;		// 先天属性
		S_ROLE_BASE_EXP				exp;		// 经验
		S_ROLE_BASE_HONOR			honor;		// 荣誉
		S_ROLE_BASE_ECON			econ;		// 经济
		S_ROLE_BASE_STATUS			status;		// 状态
		S_ROLE_BASE_LIFE			life;		// 生命
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

	//道具
	struct  S_ROLE_PROP_BASE
	{
		this_constructor(S_ROLE_PROP_BASE)

		s32    id;         //道具ID
		u8     sourcefrom; //来源
		u8     type;       //类型
		u16    count;      //数量
		s64    createtime; //创建时间
		s64    uniqueid;   //唯一id
		u32    money;      //出售价格

	};
	// 装备道具的属性
	struct  S_ROLE_PROP_EQUIP
	{
		this_constructor(S_ROLE_PROP_EQUIP)
		//随机生成的属性
		u16     p_atk;//物理攻击
		u16     m_atk;//魔法攻击
		u16     p_defend;//物理防御
		u16     m_defend;//魔法防御
		u8      crit; //暴击
		u8      dodge;//闪避
		u16     hp;   //生命
		u16		mp;
		char    nick[USER_MAX_NICK];//打造装备者的昵称
	};

	//道具上面的宝石
	struct  S_ROLE_PROP_GEM
	{
		this_constructor(S_ROLE_PROP_GEM)
		//宝石是增加固定的属性
		u8     gem_purple; //紫宝石数量   +物理攻击
		u8     gem_blue;   //蓝宝石数量   +魔法攻击
		u8     gem_yellow; //黄宝石数量   +物理防御
		u8     gem_green;  //绿宝石数量   +魔法防御
		u8     gem_red;    //红宝石数量   +生命
		inline u8 GemNum()
		{
			return gem_purple + gem_blue + gem_yellow + gem_green + gem_red;
		}
	};
	struct  S_ROLE_PROP_TEMP
	{
		void*     script_prop;//脚本指针地址
	};
	struct  S_ROLE_PROP
	{
		this_constructor(S_ROLE_PROP)

		S_ROLE_PROP_BASE   base;  //基础属性
		S_ROLE_PROP_EQUIP  equip; //装备属性
		S_ROLE_PROP_GEM    gem;   //宝石属性
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
		// 验证道具有效性(顺便会赋值脚本地址)
		bool isPropValid();
	};

	//角色战斗装备 人物属性面板穿戴的装备
	struct S_ROLE_STAND_COMBAT
	{
		this_constructor(S_ROLE_STAND_COMBAT)
		S_ROLE_PROP   equip[USER_MAX_EQUIP];
		inline void Reset() { memset(this, 0, sizeof(S_ROLE_STAND_COMBAT)); }
	};

	// 技能
	struct S_ROLE_STAND_SKILL_BASE
	{

		u32							id;
		u8							level;
		u8							quick_index;		// 快捷栏下标
		inline bool isT()
		{
			if (id > 0 && level > 0) return true;
			return false;
		}
	};

	struct S_ROLE_STAND_SKILL
	{
		this_constructor(S_ROLE_STAND_SKILL);
		S_ROLE_STAND_SKILL_BASE		skill[MAX_SKILL_COUNT];				//拥有的技能
	};

	// 背包
	struct S_ROLE_STAND_BAG
	{
		this_constructor(S_ROLE_STAND_BAG)

		u8   num;		// 当前背包数量 最大150，新手64个。其实一开始就创建了150个的空间，但不直接给出150个
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

	//202104 精灵攻击结构 适用玩家 怪物 NPC 宠物
	struct S_SPRITE_ATTACK
	{
		this_constructor(S_SPRITE_ATTACK)
		s32   hp;
		s32   mp;
		s32   p_atk;//物理攻击
		s32   m_atk;//魔法攻击
		s32   p_defend;//物理防御
		s32   m_defend;//魔法防御
		s32   dodge;//闪避
		s32   crit; //暴击
		s32   dechp;//伤害减免
		s32   speed;//移动速度
		inline void reset()
		{
			memset(this, 0, sizeof(S_SPRITE_ATTACK));
		}
	};


	// 2. 角色功能数据
	struct S_ROLE_STAND
	{
		this_constructor(S_ROLE_STAND);
		S_ROLE_STAND_COMBAT			combat;		// 角色战斗装备
		S_ROLE_STAND_BAG			bag;		// 背包
		S_ROLE_STAND_SKILL			myskill;	// 技能
		S_SPRITE_ATTACK				atk;		// 玩家基础攻击属性值
		// S_ROLE_STAND_TASK		task;		// 任务

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

	// 临时数据
	struct S_USER_TEMP_BASE
	{
		s8			line;
		s32			userindex;			// DB在线玩家下标索引
		s32			user_connectid;		// 网关连接ID
		s32			server_connectid;	// 客户端连接ID
		s32			server_clientid;	// 客户端ID

		u32			temp_SaveTime;		// 检查保存时间
		u32			temp_HeartTime;		// 心跳时间
// 		s32			temp_CopyIndex;		// 副本房间索引
		s32			temp_TeamIndex;		// 队伍房间索引
		u8			temp_MateIndex;		// 在队伍中成员位置

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
//队伍数据
//队伍成员数据 其他数据都可以通过userindex查找的到
	struct S_TEAM_MATE
	{
		bool  isused;     //是否使用
		bool  isleader;   //是不是队长 
		u8    mateindex;  //在队伍中的位置
		u32   userindex;  //玩家在服务器中的位置
		u64   memid;      //玩家唯一ID

		inline bool isT()
		{
			return memid > 10000;
		}
		inline void reset()
		{
			memset(this, 0, sizeof(S_TEAM_MATE));
		}
	};

	//队伍数据
	struct S_TEAM_BASE
	{
		bool  isused;//是否使用
		s32   index; //队伍索引
		s64   createtime;//队伍创建时间

		S_TEAM_MATE    mates[C_TEAM_PLAYERMAX];//全部成员

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

	//各个服务器性能管理
	struct S_SERVER_MANAGER
	{
		u8   state;//服务器是否连接
		u8   line; //所在线
		s32  id; //服务器ID
		s32  memory;//内存值
		s32  online;//在线人数
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