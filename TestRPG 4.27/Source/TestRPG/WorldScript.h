#ifndef  ____WORLDSCRIPT_H
#define  ____WORLDSCRIPT_H

#include "UserData.h"
#include <vector>

#define MAX_SKILLSCRIPT_COUNT		20000
#define MAX_BUFFSCRIPT_COUNT		100
#define MAX_MONSTERSCRIPT_COUNT		30000
#define MAX_MAP_COUNT				50

namespace script
{
	struct SCRIPT_SKILL_BASE 
	{
		// this_constructor(SCRIPT_SKILL_BASE);
		SCRIPT_SKILL_BASE()
		{
			memset(this, 0, sizeof(SCRIPT_SKILL_BASE));
		}

		int32		id;				// 技能ID
		int32		level;			// 技能等级
		int32		maxlevel;		// 最大技能等级
		int32		type;			// 1.自身 2.打点或者目标 3.打方向
		int32		islock;			// 是否需要锁定  0不需	1需要
		int32		isfollow;		// 是否跟随玩家移动
		int32		needjob;		// 需要职业
		int32		needlevel;		// 需要玩家角色等级
		int32		needmp;			// 需要魔法值
		int32		needhp;			// 需要生命值
		int32		coldtime;		// 冷却时间
		int32		distance;		// 攻击距离
		int32		radius;			// 半径 cm为单位
		int32		damagehp;		// 伤害值
		int32		factorhp;		// 伤害系数
		int32		runingtime;		// 运行时间
		int32		flytime;		// 技能飞行时间
		int32		waittime;		// 间隔多久计算一次伤害
		int32		continued;		// 是否持续（持续性技能）
		int32		damagecount;	// 伤害总数
		int32		buff_id;		// 会生成的buff_id
		int32		buff_percent;	// 生成buff的百分比
		FString		name;
	};

	struct SCRIPT_BUFF_BASE
	{
		int32		id;				// buff_id 
		int32		state;			// 状态
		int32		type;			// 0使用物品产生的，1使用技能产生的，2其他
		int32		runningtime;
		// 增益
		int32		add_hp;			// 生命 百分比
		int32		add_defend;		// 防御 百分比
		int32		add_atk;		// 攻击 百分比
		// 减益
		int32		subtract_hp;	// 掉血
		FString		name;
	};

	struct SCRIPT_DROP_ID
	{
		int32  id;//掉落ID
		int32  probability;//万分比
		int32  count;//掉落数量
	};

	struct SCRIPT_MONSTER_BASE
	{
		int32 id;
		int32 level;
		int32 maxhp;
		int32 maxmp;
		int32 model_id;
		int32 atk;
		int32 def;
		int32 dodge;			// 闪避
		int32 crit;				// 暴击
		int32 movespeed;
		int32 exp;
		int32 time_death;
		int32 time_fresh;
		uint8  range;			// 巡逻范围
		uint8  alert;			// 警戒范围
		uint8  isatk;
		uint8  isboss;

		FString nick;
		std::vector<int32> skill_ids;
		std::vector<SCRIPT_DROP_ID*> drop_ids;
	};

	//9、地图脚本
	struct SCRIPT_MAP_BASE
	{
		int32     isreborn;
		int32    mapid;
		uint8    maptype;
		int16    row;				//	出生点坐标x
		int16    col;				//	出生点坐标y
		uint8    copy_player;		//	副本最大人数上限
		uint8    copy_level;		//	等级要求
		uint8    copy_difficulty;	//	难度
		uint8    copy_win;			//	胜利条件
		int32    copy_limittime;	//	限制时间

		FString mapname;
	};

	extern void initScript();
	extern SCRIPT_SKILL_BASE* fingScript_Skill(int32 id, int32 level);
	extern SCRIPT_BUFF_BASE* fingScript_Buff(int32 id);
	extern SCRIPT_MONSTER_BASE* findScript_Monster(int32 id);
	extern SCRIPT_MAP_BASE* findScript_Map(int32 id);
}



#endif