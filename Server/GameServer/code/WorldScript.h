#ifndef  ____WORLDSCRIPT_H
#define  ____WORLDSCRIPT_H

#include "UserData.h"

#define MAX_SKILLSCRIPT_COUNT		20000
#define MAX_BUFFSCRIPT_COUNT		100
#define MAX_MONSTERSCRIPT_COUNT		30000
#define MAX_LEVELSCRIPT_COUNT		1000
#define MAX_TASKSCRIPT_COUNT		30000

namespace script
{
	struct SCRIPT_SKILL_BASE 
	{
		this_constructor(SCRIPT_SKILL_BASE);

		s32		id;				// 技能ID
		s32		level;			// 技能等级
		s32		maxlevel;		// 最大技能等级
		s32		type;			// 1.自身 2.打点或者目标 3.打方向
		s32		islock;			// 是否需要锁定  0不需	1需要
		s32		isfollow;		// 是否跟随玩家移动
		s32		needjob;		// 需要职业
		s32		needlevel;		// 需要玩家角色等级
		s32		needmp;			// 需要魔法值
		s32		needhp;			// 需要生命值
		s32		coldtime;		// 冷却时间
		s32		distance;		// 攻击距离
		s32		radius;			// 半径 cm为单位
		s32		damagehp;		// 伤害值
		s32		factorhp;		// 伤害系数
		s32		runingtime;		// 运行时间
		s32		flytime;		// 技能飞行时间
		s32		waittime;		// 间隔多久计算一次伤害
		s32		continued;		// 是否持续（持续性技能）
		s32		damagecount;	// 伤害总数
		s32		buff_id;		// 会生成的buff_id
		s32		buff_percent;	// 生成buff的百分比
	};

	struct SCRIPT_BUFF_BASE
	{
		this_constructor(SCRIPT_BUFF_BASE);

		s32		id;				// buff_id 
		s32		state;			// 状态
		s32		type;			// 0使用物品产生的，1使用技能产生的，2其他
		s32		runningtime;
		// 增益
		s32		add_hp;			// 生命 百分比
		s32		add_defend;		// 防御 百分比
		s32		add_atk;		// 攻击 百分比
		// 减益
		s32		subtract_hp;	// 掉血
	};

	//4、玩家等级 属性表
	struct  SCRIPT_PALYER_LEVEL
	{
		s32   level;
		s32   exp;
		s32   hp;
		s32   mp;
		s32   p_atk;
		s32   m_atk;
		s32   p_defend;
		s32   m_defend;
		s32   hit;  //命中
		s32   dodge;//闪避
		s32   crit; //暴击
		s32   dechp;//伤害减免
		s32   speed;//移动速度

	};

	struct SCRIPT_DROP_ID
	{
		this_constructor(SCRIPT_DROP_ID);
		s32  id;//掉落ID
		s32  probability;//万分比
		s32  count;//掉落数量
	};

	struct SCRIPT_MONSTER_BASE
	{
		this_constructor(SCRIPT_MONSTER_BASE);
		u32 id;
		u32 level;
		u32 maxhp;
		u32 maxmp;
		// u32 model_id;
		u32 atk;
		u32 def;
		u32 dodge;
		u32 crit;
		u32 movespeed; 
		u32 exp; 
		u32 time_death; 
		u32 time_fresh; 
		u8  range;			// 巡逻范围
		u8  alert;			// 警戒范围
		u8  isatk; 
		u8  isboss; 

		std::vector<s32> skill_ids;
		std::vector<SCRIPT_DROP_ID*> drop_ids;
	};

	//6、装备表
	struct  SCRIPT_PROP_EQUIP
	{
		s32   id;   //唯一标记
		s32   kind; //分类
		u8   level;//装备等级
		u8   color;//品质
		u8   job;  //职业
		u8   part; //装备位置

		s32   hp;//生命
		s32   mp;//魔法
		s32   p_atk;//物理攻击
		s32   m_atk;//魔法攻击
		s32   p_defend;//物理防御
		s32   m_defend;//魔法防御
		s32   dodge;//闪避
		s32   crit; //暴击

		s32   r_atk;
		s32   r_defend;
		s32   r_dodge;
		s32   r_crit;
		s32   r_hp;

		s32   needgold;    //打造装备需要金币
		s32   needpropid_1;//打造装备需要道具ID1
		s32   needpropnum_1;//需要数量1
		s32   needpropid_2;
		s32   needpropnum_2;

		inline void reset()
		{
			memset(this, 0, sizeof(SCRIPT_PROP_EQUIP));
		}
	};
	//7、消耗道具表
	struct  SCRIPT_PROP_CONSUME
	{
		s32   id;   //唯一标记
		s32   kind; //分类
		u16   maxcount;//最大叠加数量

		s32   potion_value; //药品数值
		s32   potion_cdtime;//药品时间

		u8   gem_class;//属性分类
		s32  gem_value;//宝石数值

		inline void reset()
		{
			memset(this, 0, sizeof(SCRIPT_PROP_CONSUME));
		}
	};

	//5、道具总表
	struct  SCRIPT_PROP
	{
		s32  id;      //唯一标记
		u8   type;    //类型
		s32  kind;    //分类
		u8   color;   //品质
		u8   minlevel;//使用最小等级
		s32  maxcount;//最大叠加数
		s32  price;   //出售价格
		u8   maxuse;       //是否批量使用
		u8   issplit;      //是否可拆分
		u8   isdestroy;    //是否可销毁
		u8   istransaction;//是否可交易
		u8   isshow;       //是否可展示
		u8   isbind;       //是否绑定
		u8   issell;       //是否可出售
		u8   tab;//背包页签

		SCRIPT_PROP_EQUIP    equip;
		SCRIPT_PROP_CONSUME  consume;
	};


	//9、地图脚本
	struct SCRIPT_MAP_BASE
	{
		this_constructor(SCRIPT_MAP_BASE);
		bool    isreborn;
		u32     mapid;
		u8      maptype;
		s16     row;				//	出生点坐标x
		s16     col;				//	出生点坐标y
		u8      copy_player;		//	副本最大人数上限
		u8      copy_level;			//	等级要求
		u8      copy_difficulty;	//	难度
		u8      copy_win;			//	胜利条件
		u32     copy_limittime;		//	限制时间
	};

	extern void initScript();
	extern SCRIPT_SKILL_BASE* findScript_Skill(s32 id, s32 level);
	extern SCRIPT_BUFF_BASE* findScript_Buff(s32 id);
	extern SCRIPT_PALYER_LEVEL* findScript_Level(s32 level);
	extern SCRIPT_MONSTER_BASE* findScript_Monster(s32 id);
	extern SCRIPT_PROP* findScript_Prop(s32 id);
	extern SCRIPT_MAP_BASE* findScript_Map(s32 id);
}


#endif