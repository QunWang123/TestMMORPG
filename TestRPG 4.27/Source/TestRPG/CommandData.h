#ifndef  __COMMANDDATA_H
#define  __COMMANDDATA_H

#include "Containers/UnrealString.h"

#define USER_MAX_MEMBER			20
#define USER_MAX_PASS			20
#define USER_MAX_KEY			11
#define USER_MAX_ROLE			3
#define USER_MAX_NICK			20

#define  CMD_10			10		// 账号登录 验证
#define  CMD_20			20		// 账号注册
#define  CMD_30			30		// 更新登录时间
#define  CMD_90			90		// 心跳

#define  CMD_100		100		// 登录游戏	获取角色数据	
#define  CMD_200		200		// 选择角色	开始游戏
#define  CMD_300		300		// 创建角色
#define  CMD_400		400		// 删除角色

#define  CMD_500		500		// 获取角色数据
#define  CMD_600		600		// 玩家掉线
#define  CMD_610		610		// save base

#define  CMD_700		700		// other roledata
#define  CMD_710		710		// updata hp
#define  CMD_720		720		// updata mp
#define  CMD_730		730		// updata state
#define  CMD_740		740		// 经验
#define  CMD_750		750		// 等级
#define  CMD_760		760		// 经济
#define  CMD_770		770		// 攻击属性

#define  CMD_800		800		// 背包道具更新
#define  CMD_810		810		// 战斗装备更新
#define  CMD_820		820		// 获取其他人的战斗装备
#define  CMD_830		830		// 获取背包数据
#define  CMD_840		840		// 整理背包
#define  CMD_850		850		// 穿上装备
#define  CMD_860		860		// 脱下装备
#define  CMD_870		870		// 出售道具
#define  CMD_880		880		// 客户端自己用的，用来给装备UI更新攻击数据
#define  CMD_890		890		// 交换格子

#define  CMD_900		900		// 开始进入世界
#define  CMD_901		901		// 切图
#define	 CMD_902		902		// 切线路
#define  CMD_1000		1000	// 自己移动
#define  CMD_1100		1100	// 其他玩家移动
#define  CMD_1200		1200	// 速度发生变化
#define  CMD_2000		2000	// 速度发生变化

#define  CMD_3000		3000	// 技能
#define  CMD_3100		3100	// 技能出招
#define  CMD_3200		3200	// 技能伤害

#define  CMD_4000		4000	// 玩家死亡
#define  CMD_4100		4100	// 收到玩家复活错误指令
#define  CMD_4200		4200	// 玩家成功复活
#define  CMD_5000		5000	// buff

#define  CMD_6000       6000	// 返回错误
#define  CMD_6100       6100	// 正常掉落
#define  CMD_6200       6200	// 删除掉落
#define  CMD_6300       6300 

#define	 CMD_8000       8000	// 怪物数据
#define	 CMD_8100       8100	// 怪物返回
#define	 CMD_8200       8200	// 怪物生命
#define	 CMD_8300       8300	// 怪物状态
#define	 CMD_8400       8400	// 怪物移动
#define	 CMD_8500       8500	// 怪物出招
#define	 CMD_8700       8700	// 怪物技能伤害
#define	 CMD_8800       8800	// 怪物BUFF

namespace app
{
	// const FString gate_ip = "127.0.0.1";
	const FString login_ip = "127.0.0.1";

	// const int gate_port = 14000;
	const int login_port = 15000;
}

#endif