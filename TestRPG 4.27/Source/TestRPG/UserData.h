// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommandData.h"
#include "GameFramework/Actor.h"
#include <map>
#include "IDefine_Udp.h"
#include "UserData.generated.h"


#define MAX_EXE_LEN					300
#define C_WORLDMAP_ONE_GRID	100	
#define C_WORLDMAP_NODE_X	10	
#define C_WORLDMAP_NODE_Y	10	

#define MAX_USER_BAG		150 //最大背包格子数量
#define MAX_USER_EQUIP		15
#define MAX_SKILL_COUNT		10
#define MAX_BUFF_COUNT		10
#define MAX_ROW				1000
#define MAX_COL				1000

class AMyActor;

UENUM(BlueprintType)
enum class E_NODE_TYPE : uint8
{
	N_FREE = 0x00,
	N_ROLE,
	N_MONSTER,
	N_NPC,
	N_PET,
	N_PROP
};

UENUM(BlueprintType)
enum class E_EVENT_TYPE : uint8
{
	EVENT_FREE = 0x00,
	EVENT_WAIT,
	EVENT_MOVE,
	EVENT_MOVE_DIR,
	EVENT_SKILL,
	EVENT_HIT,
	EVENT_TOSRC,		// 拉回原点
	EVENT_TRANS,
	EVENT_EXIT,
	EVENT_DEAD,
};

UENUM(BlueprintType)
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



#pragma pack(push, packing)
#pragma pack(1)

// 游戏账号，角色基础数据，主要用于选择角色使用
USTRUCT(BlueprintType)
struct FUSER_MEMBER_ROLE
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_MEMBER_ROLE")
		int64			id;					// 角色ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_MEMBER_ROLE")
		uint8			job;				// 职业
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_MEMBER_ROLE")
		uint8			sex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_MEMBER_ROLE")
		int32			clothid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_MEMBER_ROLE")
		int32			weaponid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_MEMBER_ROLE")
		int32			level;				// 这里和服务器不一样，是因为蓝图中没有16位的
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_MEMBER_ROLE")
		FString			nick;

	inline void reset()
	{
		FMemory::Memset(this, 0, sizeof(FUSER_MEMBER_ROLE));
	}
};

/*自定义的一些结构体*/
// 服务器处理int类型，处理浮点会降低性能
USTRUCT(BlueprintType)
struct FS_VECTOR
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VECTOR")
		int32 x;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VECTOR")
		int32 y;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VECTOR")
		int32 z;
};

/*********************************************************************************************/
/*********************************************************************************************/
// 经验
USTRUCT(BlueprintType)
struct FROLE_BASE_EXP
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_EXP")
		int32				level;		// 等级
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_EXP")
		int32				currexp;	// 当前经验值
};

USTRUCT(BlueprintType)
struct FROLE_BASE_ECON
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_ECON")
		int32				gold;		// 金币
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_ECON")
		int32				diamonds;	// 钻石
};

USTRUCT(BlueprintType)
struct FROLE_BASE_STATUS
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_STATUS")
		uint8				state;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_STATUS")
		int32				face;		// 朝向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_STATUS")
		int32				mapid;		// 当前所在地图ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_STATUS")
		FS_VECTOR			pos;		// 当前坐标
};

USTRUCT(BlueprintType)
struct FROLE_BASE_LIFE
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_LIFE")
		int32				hp;			// 当前生命值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE_LIFE")
		int32				mp;			// 当前魔法值
};

// USTRUCT(BlueprintType)
// struct FROLE_BASE_HONOR
// {
// 	GENERATED_BODY()
// 	u32				medalnum;	// 奖牌数量
// };


/*********************************************************************************************/
USTRUCT(BlueprintType)
struct FROLE_BASE
{
	GENERATED_BODY()

// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE")
// 		FROLE_BASE_INNATE			innate;		// 先天属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE")
		FROLE_BASE_EXP			exp;		// 经验
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE")
// 		FROLE_BASE_HONOR			honor;		// 荣誉
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE")
		FROLE_BASE_ECON			econ;		// 经济
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE")
		FROLE_BASE_STATUS		status;		// 状态
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BASE")
		FROLE_BASE_LIFE			life;		// 生命
	inline void reset()
	{
		FMemory::Memset(this, 0, sizeof(FROLE_BASE));
	}
};


//道具
USTRUCT(BlueprintType)
struct  FROLE_PROP_BASE
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_BASE")
		int32    id;         //道具ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_BASE")
		uint8     sourcefrom; //来源
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_BASE")
		uint8     type;       //类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_BASE")
		int32    count;      //数量(原u16位)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_BASE")
		int64    createtime; //创建时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_BASE")
		int64    uniqueid;   //唯一id
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_BASE")
		int32    money;      //出售价格
};
// 装备道具的属性
USTRUCT(BlueprintType)
struct  FROLE_PROP_EQUIP
{
	GENERATED_BODY()
	//随机生成的属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_EQUIP")
		int32     p_atk;//物理攻击（原16位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_EQUIP")
		int32     m_atk;//魔法攻击（原16位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_EQUIP")
		int32     p_defend;//物理防御（原16位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_EQUIP")
		int32     m_defend;//魔法防御（原16位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_EQUIP")
		uint8      crit; //暴击
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_EQUIP")
		uint8      dodge;//闪避
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_EQUIP")
		int32     hp;   //生命（原16位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_EQUIP")
		int32     mp;   //生命（原16位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_EQUIP")
		FString	  nick;//打造装备者的昵称
};

//道具上面的宝石
USTRUCT(BlueprintType)
struct  FROLE_PROP_GEM
{
	GENERATED_BODY()
	//宝石是增加固定的属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_GEM")
		uint8     gem_purple; //紫宝石数量   +物理攻击
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_GEM")
		uint8     gem_blue;   //蓝宝石数量   +魔法攻击
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_GEM")
		uint8     gem_yellow; //黄宝石数量   +物理防御
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_GEM")
		uint8     gem_green;  //绿宝石数量   +魔法防御
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PROP_GEM")
		uint8     gem_red;    //红宝石数量   +生命
	inline uint8 GemNum()
	{
		return gem_purple + gem_blue + gem_yellow + gem_green + gem_red;
	}
};

USTRUCT(BlueprintType)
struct FROLE_PROP
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_PROP")
		FROLE_PROP_BASE   base;  //基础属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_PROP")
		FROLE_PROP_EQUIP  equip; //装备属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_PROP")
		FROLE_PROP_GEM    gem;   //宝石属性

	inline void reset()
	{
		FMemory::Memset(this, 0, sizeof(FROLE_PROP));
		// memset(this, 0, sizeof(FROLE_PROP));
		// temp.script_prop = NULL;
	}
	inline bool IsT()
	{
		if (base.id > 0 && base.count > 0)
		{
			return true;
		}
		return false;
	}
	inline bool isSame(FROLE_PROP* prop)
	{
		if (this->base.id != prop->base.id || this->base.type != prop->base.type ||
			this->base.money != prop->base.money)
			return false;
		return true;
	}
// 	inline uint32 sendSize()
// 	{
// 		uint32 size = sizeof(FROLE_PROP_BASE) + sizeof(FROLE_PROP_EQUIP) + sizeof(FROLE_PROP_GEM);
// 		return size;
// 	}
};

USTRUCT(BlueprintType)
struct FROLE_STAND_COMBAT
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "COMBAT")
		TArray<FROLE_PROP>				equips;
	void reset()
	{
		if (equips.Num() == MAX_USER_EQUIP)
		{
			for (int i = 0; i < MAX_USER_EQUIP; i++)
			{
				equips[i].reset();
			}
			return;
		}
		for (int i = 0; i < MAX_USER_EQUIP; i++)
		{
			FROLE_PROP b;
			b.reset();
			equips.Push(b);
		}
	}
};

USTRUCT(BlueprintType)
struct FROLE_STAND_BAG
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BAG")
		uint8							num;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BAG")
		TArray<FROLE_PROP>				bags;
	void reset()
	{
		num = 0;
		if (bags.Num() == MAX_USER_BAG)
		{
			for (int i = 0; i < MAX_USER_BAG; i++)
			{
				bags[i].reset();
			}
			return;
		}
		for (int i = 0; i < MAX_USER_BAG; i++)
		{
			FROLE_PROP b;
			b.reset();
			bags.Push(b);
		}
	}
};

// 技能
USTRUCT(BlueprintType)
struct FROLE_STAND_SKILL_BASE
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SKILL_BASE")
		int32 id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SKILL_BASE")
		uint8 level;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SKILL_BASE")
		uint8 quick_level;

	inline bool isT()
	{
		if (id > 0 && level > 0) return true;
		return false;
	}

	inline void reset()
	{
		FMemory::Memset(this, 0, sizeof(FROLE_STAND_SKILL_BASE));
	}
};

USTRUCT(BlueprintType)
struct FROLE_STAND_SKILL
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SKILL")
		TArray<FROLE_STAND_SKILL_BASE> skill;

	void reset()
	{
		if (skill.Num() == MAX_SKILL_COUNT)
		{
			for (int i = 0; i < MAX_SKILL_COUNT; i++)
			{
				skill[i].reset();
			}
			return;
		}
		for (int i = 0; i < MAX_SKILL_COUNT; i++)
		{
			FROLE_STAND_SKILL_BASE s;
			s.reset();
			skill.Push(s);
		}
	}
};

// 角色功能数据
USTRUCT(BlueprintType)
struct FROLE_STAND
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STAND_COMBAT")
		FROLE_STAND_COMBAT			combat;		// 角色战斗装备
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STAND_BAG")
		FROLE_STAND_BAG				bag;		// 背包
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STAND_SKILL")
		FROLE_STAND_SKILL			myskill;	// 技能
		// S_ROLE_STAND_TASK		task;		// 任务
	inline void reset()
	{
		// FMemory::Memset(this, 0, sizeof(FROLE_BASE));
		myskill.reset();
		bag.reset();
		combat.reset();
	}
};

USTRUCT(BlueprintType)
struct FLOCK_ATK_DATA
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOCK_ATK_DATA")
		int32				lock_SkillID;			// 锁定技能ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOCK_ATK_DATA")
		uint8				lock_Type;				// 锁定类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOCK_ATK_DATA")
		int32				lock_Index;				// 锁定目标下标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOCK_ATK_DATA")
		FVector				lock_Targetpos;			// 锁定目标坐标

	inline void reset()
	{
		FMemory::Memset(this, 0, sizeof(FLOCK_ATK_DATA));
	}
};


// BUFF
USTRUCT(BlueprintType)
struct FUSER_ROLE_BUFF_BASE
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BUFF_BASE")
		int32 buff_id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BUFF_BASE")
		int32 runningtime;
	inline void reset()
	{
		FMemory::Memset(this, 0, sizeof(FUSER_ROLE_BUFF_BASE));
	}
};

// BUFF
USTRUCT(BlueprintType)
struct FUSER_ROLE_BUFF
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BUFF")
		int32 checkTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BUFF")
		int32 state;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROLE_BUFF")
		TArray<FUSER_ROLE_BUFF_BASE>	data;

	inline void reset()
	{
		checkTime = 0;
		state = 0;
		if (data.Num() == MAX_BUFF_COUNT)
		{
			for (int32 i = 0; i < MAX_BUFF_COUNT; i++)
			{
				data[i].reset();
			}
		}
		else
		{
			for (int32 i = 0; i < MAX_BUFF_COUNT; i++)
			{
				FUSER_ROLE_BUFF_BASE d;
				d.reset();
				data.Push(d);
			}
		}
	};
	inline FUSER_ROLE_BUFF_BASE* findFree()
	{
		for (int32 i = 0; i < MAX_BUFF_COUNT; i++)
		{
			if (data[i].buff_id <= 0)
			{
				return &data[i];
			}
		}
		return nullptr;
	}
	inline bool isMove()
	{
		int32 value = state & E_BUFF_NOMOVE;
		if (value > 0) return false;
		return true;
	}
	inline bool isUseSkill()
	{
		int32 value = state & E_BUFF_NOSKILL;
		if (value > 0) return false;
		return true;
	}
	inline void	addState(int32 value)
	{
		state = state | value;
	}
	inline void	removeState(int32 value)
	{
		state = state & (~value);
		// state = state ^ value;
	}
};

USTRUCT(BlueprintType)
struct FSPRITE_ATTACK
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   hp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   mp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   p_atk;//物理攻击
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   m_atk;//魔法攻击
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   p_defend;//物理防御
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   m_defend;//魔法防御
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   dodge;//闪避
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   crit; //暴击
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   dechp;//伤害减免
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATTACK")
		int32   speed;//移动速度
	inline void reset()
	{
		FMemory::Memset(this, 0, sizeof(FSPRITE_ATTACK));
	}
};

USTRUCT(BlueprintType)
struct FUSER_TEMP
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_TEMP")
		int32				skillPublicTime;	// 技能公共CD时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_TEMP")
		TArray<int32>		skillColdTime;		// 技能冷却时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_TEMP")
		FLOCK_ATK_DATA		lock_Atk;			// 锁定攻击数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_TEMP")
		FUSER_ROLE_BUFF		buff_run;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_TEMP")
		int32				skillUseTime;
	inline void reset()
	{
		skillPublicTime = 0;
		lock_Atk.reset();
		buff_run.reset();
		if (skillColdTime.Num() == MAX_SKILL_COUNT)
		{
			for (int i = 0; i < MAX_SKILL_COUNT; i++)
			{
				skillColdTime[i] = 0;
			}
		}
		else
		{
			for (int i = 0; i < MAX_SKILL_COUNT; i++)
			{
				skillColdTime.Push(0);
			}
		}
	}
};

USTRUCT(BlueprintType)
struct FUSER_ROLE_EVENT
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_EVENT")
		bool isEventing;						// 是否正处在事件中
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_EVENT")
		uint8 event_Type;						// 事件类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_EVENT")
		FLOCK_ATK_DATA event_LockAtk;			// 事件锁定攻击数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_EVENT")
		TArray<FString> event_List;				// 事件列表

	inline void reset()
	{
		isEventing = false;
		event_Type = 0;
		event_LockAtk.reset();
		while (event_List.Num() > 0)
		{
			event_List.Pop();
		}
	};

};

USTRUCT(BlueprintType)
struct FUSER_ROLE_BASE
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_BASE")
		uint8				select_roleindex;	// 选择的角色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_BASE")
		int64				memid;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_BASE")
		int32				userindex;			// 玩家在服务器上的索引
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_BASE")
		FUSER_MEMBER_ROLE	rolebase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_BASE")
		FROLE_BASE			base;				// 角色基础数据（血量、荣誉等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_BASE")
		FROLE_STAND			stand;				// 角色功能数据（背包技能等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_BASE")
		FSPRITE_ATTACK		atk;				// 玩家基础攻击属性值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_BASE")
		FUSER_TEMP			tmp;				// 角色临时数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USER_ROLE_BASE")
		FUSER_ROLE_EVENT	events;				// 事件列表
	inline void reset() 
	{
		userindex = 0;
		rolebase.reset();
		base.reset();
		stand.reset();
		tmp.reset();
		events.reset();
	}
};

USTRUCT(BlueprintType)
struct FOTHER_ROLE_BASE
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		int32				userindex;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		int64				roleid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		uint8				job;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		uint8				sex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		int32				level;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		int32				face;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		int32				hp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		FVector				pos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		FString				nick;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		FUSER_ROLE_BUFF		buff_run;
	void* view;			// 显示对象的引用

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		FUSER_ROLE_EVENT	events;

	// 装备数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		FROLE_STAND_COMBAT	combat;
	// 攻击数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OTHER_ROLE_BASE")
		FSPRITE_ATTACK		atk;
	// buff数据等

	inline void reset()
	{
		view = NULL;
		events.reset();
		buff_run.reset();
		combat.reset();
		atk.reset();
	}

};

// 格子坐标
USTRUCT(BlueprintType)
struct FGRID_BASE
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "S_GRID_BASE")
		int32 row;		// 行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "S_GRID_BASE")
		int32 col;		// 列	
};
USTRUCT(BlueprintType)
struct FMAP_DATA
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "S_WORLD_MAP")
		FVector	leftpos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "S_WORLD_MAP")
		int32 row;		// 行
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "S_WORLD_MAP")
		int32 col;		// 列	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "S_WORLD_MAP")
		int32 mapID;
};

struct S_MAP_BIN_BASE
{
	FS_VECTOR left_pos;
	int row;
	int col;
	uint8 value[MAX_ROW][MAX_COL];

	inline void reset()
	{
		memset(this, 0, MAX_ROW * MAX_COL);
	}
};

USTRUCT(BlueprintType)
struct  FDROP_BASE
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		int32 ftime;//锁定时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		int32 lock_index;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		int32 lock_teamid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		int32 dropindex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		int32 dropid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		FGRID_BASE  grid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		FVector pos;
};

struct  S_SEND_3000 :public net::S_DATA_HEAD
{
	int8  skillindex;
	int8  lock_type;
	int32  lock_index;
	FS_VECTOR   targetpos;
};

struct  S_RECV_3100 :public net::S_DATA_HEAD
{
	int32 userindex;
	int32 skillid;
	int8  level;
	int8  skillindex;
	int8  lock_type;//怪物索引
	int32 lock_index;
	FS_VECTOR   targetpos;
};

#pragma pack(pop, packing)


extern char FileExePath[MAX_EXE_LEN];
extern int32				delaytime;						// 延迟时间，用来算ping得
extern uint8				loginkey[USER_MAX_KEY];			// 登录密钥
extern FUSER_MEMBER_ROLE	rolebase[USER_MAX_ROLE];		// 角色基础数据
extern FUSER_ROLE_BASE		__myUserData;					// 主控角色数据
extern FMAP_DATA __CurMapInfo;
extern S_MAP_BIN_BASE __MapBin;
extern TMap<int32, FOTHER_ROLE_BASE> onLineDatas;
extern void renderUserEvent();
extern void renderUserBuff();
extern void updateSkillColdTime();
extern void clearAllUser();
extern void InitMapInfo(int mapid);




//********************************************************
//********************************************************
//********************************************************
//地图编辑器部分.....................................

extern FVector __LeftPos; //左上角坐标
extern S_MAP_BIN_BASE   __MapBin;//地图数据
extern int32 __EditorState;//当前状态
extern std::map<int32, AMyActor*> __MyActor;
extern void insertBinValue(int row, int col, int id);
extern void deleteBinValue(int row, int col);
extern void deleteBinValue(int id);

extern void writeBinData(char* filename);//写2进制数据
extern void readBinData(char* filename);//读2进制数据



UCLASS()
class TESTRPG_API AUserData : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUserData() { PrimaryActorTick.bCanEverTick = false; };

};
