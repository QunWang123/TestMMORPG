#ifndef  ____COMMANDDATA_H
#define  ____COMMANDDATA_H

#include "UserData.h"
#include "IDefine_Udp.h"

#define  CMD_10			10		// 账号登录 验证
#define  CMD_20			20		// 账号注册
#define  CMD_30			30		// 更新登录时间
#define  CMD_40         40		//更新网关人数
#define  CMD_90			90		// 心跳

#define  CMD_100		100		// 登录游戏	获取角色数据	
#define  CMD_200		200		// 选择角色	开始游戏
#define  CMD_300		300		// 创建角色
#define  CMD_400		400		// 删除角色

#define  CMD_500		500		// 获取角色数据
#define  CMD_600		600		// 玩家掉线
#define  CMD_601        601     // 玩家掉线 更新下账号下面的mini角色数据
#define  CMD_602        602     // DB与game断开连接 通知gate
#define  CMD_610		610		// save base

#define  CMD_700		700		// other roledata
#define  CMD_710		710		// updata hp
#define  CMD_720		720		// updata mp
#define  CMD_730		730		// updata state
#define  CMD_731        731     // 状态（坐标） 
#define  CMD_740		740		// 经验
#define  CMD_750		750		// 等级
#define  CMD_760		760		// 经济
#define  CMD_770		770		// 攻击属性


#define  CMD_800		800		// 背包道具保存
#define  CMD_810		810		// 战斗装备保存
#define  CMD_820		820		// 获取战斗装备（自己的和他人都从这里获取）
#define  CMD_830		830		// 获取背包数据
#define  CMD_840		840		// 整理背包
#define  CMD_850		850		// 穿上装备
#define  CMD_860		860		// 脱下装备
#define  CMD_870		870		// 出售道具
#define	 CMD_890		890		// 交换装备格子

#define  CMD_900		900		// 开始进入世界
#define  CMD_901		901		// 切换地图
#define  CMD_902		902		// 换线

#define  CMD_1000		1000	// 自己移动
#define  CMD_1100		1100	// 其他玩家移动
#define  CMD_1200		1200	// 速度发生变化
#define  CMD_2000		2000	// 聊天

#define  CMD_3000		3000	// 技能
#define  CMD_3100		3100	// 技能出招
#define  CMD_3200		3200	// 技能伤害

#define  CMD_4000		4000	// 玩家死亡
#define  CMD_4100		4100	// 收到玩家复活错误指令
#define  CMD_4200		4200	// 玩家成功复活
#define  CMD_5000		5000	// buff

#define  CMD_6000		6000	// 拾取掉落
#define  CMD_6100		6100 	// 掉落物品
#define  CMD_6200		6200 	// 掉落删除
#define  CMD_6300		6300	// 掉落

#define  CMD_7000		7000	// 组队-创建队伍
#define  CMD_7010		7010	// 组队-邀请入队
#define  CMD_7020		7020	// 组队-同意加入队伍
#define  CMD_7030		7030	// 组队-拒绝加入队伍
#define  CMD_7040		7040	// 组队-队友离开
#define  CMD_7050		7050	// 组队-队长踢人
#define  CMD_7060		7060	// 组队-申请加入队伍
#define  CMD_7070		7070	// 组队-申请加入 队长同意
#define  CMD_7080		7080	// 组队-申请加入 队长拒绝
#define  CMD_7090		7090	// 组队-队长转让
#define  CMD_7100		7100	// 更新队伍成员数据
#define  CMD_7110		7110	// 广播xx成为了队长
#define  CMD_7120		7120	// 自己加入了队伍

#define	 CMD_7200		7200	// 开启单人副本
#define	 CMD_7300		7300	// 开启多人副本
#define	 CMD_7301		7301	// 进入副本 
#define	 CMD_7400		7400	// 离开副本
#define	 CMD_7401		7401	// 副本已解散

#define  CMD_7500		7500	// 更新 给游戏服务器 组队数据
#define  CMD_7600		7600	// 更新 给游戏服务器 玩家的队伍数据

#define	 CMD_8000		8000	// 申请怪物数据
#define	 CMD_8100		8100	// 怪物返回
#define	 CMD_8200		8200	// 怪物生命
#define	 CMD_8300		8300	// 怪物状态
#define	 CMD_8400		8400	// 怪物移动
#define	 CMD_8500		8500	// 怪物出招
#define	 CMD_8700		8700	// 怪物技能伤害
#define	 CMD_8800		8800	// 怪物BUFF

namespace app
{
#pragma pack(push, packing)
#pragma pack(1)
	struct S_LOGIN_GAME
	{
		this_constructor(S_LOGIN_GAME);

		u8				line;//选择的线路
		u32				ServerConnectID;			// 中心服务器上的连接ID（其实也就是网关连在中心上的ID）
		u32				ServerClientID;				// 中心服务器上的客户端ID（其实也就是网关连在中心上的ID）
		u32				UserConnectID;				// 网关服务器上的连接ID（其实也就是用户连在网关上的ID）
		u32				UserClientID;				// 网关服务器上的客户端ID（其实也就是用户连在网关上的ID）
		char			name[USER_MAX_MEMBER];		// 账号
		char			key[USER_MAX_KEY];			// 密钥
	};

	struct S_COMMAND_BASE
	{
		u32				server_connectid;			// 服务器上的连接ID
		u32				server_clientid;			// 中心服务器上的客户端ID
		u32				user_connectid;				// 网关服务器上的连接ID
		u64				memid;						// 账号id
	};

	struct S_COMMAND_GATEBASE
	{
		u32				user_connectid;				// 网关服务器上的连接ID
		u64				memid;						// 账号id
	};

	struct S_SELECT_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_SELECT_ROLE);

		u8				roleindex;					// 角色索引
		u32				userindex;					// 玩家索引位置，DB服务器赋值
		u32				mapid;						// 地图id，DB服务器赋值
		inline void reset() { memset(this, 0, sizeof(S_SELECT_ROLE)); };
	};

	struct S_CREATE_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_CREATE_ROLE);

		u64				roleid;						// 角色ID
		u8				roleindex;					// 角色索引
		u8				job;						// 职业
		u8				sex;						// 性别
		char			nick[USER_MAX_NICK];		// 昵称
	};

	struct S_DELETE_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_DELETE_ROLE);

		u64				roleid;						// 角色ID
		u8				roleindex;					// 角色索引
	};

	struct S_LOAD_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_LOAD_ROLE);

		u16 cmd;
		u32 userindex;								// DB服务器上缓存的玩家数据索引值
		u32 mapid;	
		u8  line;
		inline void reset() { memset(this, 0, sizeof(S_LOAD_ROLE)); };

	};

	// 进入世界数据
	struct S_ENTRY_BASE : S_COMMAND_BASE
	{
		this_constructor(S_ENTRY_BASE);

		u32 userindex;
		inline void reset() { memset(this, 0, sizeof(S_ENTRY_BASE)); };
	};

	struct S_UPDATE_VALUE : S_COMMAND_BASE
	{
		this_constructor(S_UPDATE_VALUE);

		s32 value;
		u32 targetindex;
		inline void reset() { memset(this, 0, sizeof(S_UPDATE_VALUE)); };
	};

	struct S_GETROLE_DATA : S_COMMAND_BASE
	{
		this_constructor(S_GETROLE_DATA);

		u32 targetindex;		//需要获取的玩家索引值
		inline void reset() { memset(this, 0, sizeof(S_GETROLE_DATA)); };
	};

	struct S_MOVE_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_MOVE_ROLE);

		u32 userindex;
		s16 face;
		s32 speed;		
		S_VECTOR3 curpos;
		S_VECTOR3 targetpos;
		inline void reset() { memset(this, 0, sizeof(S_MOVE_ROLE)); };
	};

	struct S_SKILL_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_SKILL_ROLE);

		u32	userindex;				// 使用技能得玩家索引
		u8	skillindex;				// 技能索引
		u8	lock_type;				// 锁定类型
		u32	lock_index;			// 锁定index
		S_VECTOR3 targetpos;		// 目标点
		inline void reset() { memset(this, 0, sizeof(S_SKILL_ROLE)); };
	};

	//获取其他人 战斗装备
	struct S_COMBAT_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_COMBAT_DATA)

			u32  userindex;
	};
	//获取背包数据
	struct S_BAG_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_BAG_DATA)

			u32  userindex;
	};

	//穿上 脱下装备 或者出售 
	struct S_EQUIPUPDOWN_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_EQUIPUPDOWN_DATA)

		u32  userindex;
		u8   pos;			// 背包或者战斗装备中的位置
		u32  propid;		// 道具ID
	};
	// 交换格子
	struct S_BAGEQUIPSWAP_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_BAGEQUIPSWAP_DATA)

		u32  userindex;
		u8   pos1;			// 背包或者战斗装备中的位置
		u32  propid1;		// 道具ID
		u8   pos2;			// 背包或者战斗装备中的位置
		u32  propid2;		// 道具ID
	};

	// *******************************************************************************
	// *******************************************************************************
	// *******************************************************************************
	// 队伍
	//创建队伍 7000
	struct S_TEAM_CREATE :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_CREATE)

			u32  userindex;
		s32  teamindex;
		u8   mateindex;
		bool isleader;
	};
	//邀请xxx加入队伍 7010
	struct S_TEAM_PLEASE :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_PLEASE)

			u32  userindex;
		s32  other_userindex;
		s64  other_memid;
		char nick[USER_MAX_NICK];
	};

	//7020 同意加入xxx的队伍
	struct S_TEAM_AGREE :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_AGREE)

			u32  userindex;
		u32  leaderindex;//队长索引
		s64  leadermemid;//队长账号id
	};
	//7040 xxx离开队伍
	struct S_TEAM_LEAVE :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_LEAVE)

			u32  userindex;
		u8   mateindex;
	};
	//7050 队长踢人
	struct S_TEAM_KILL :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_KILL)

			u32  userindex;
		u8   mateindex;
	};
	//7060 申请加入xx队伍
	struct S_TEAM_REQUEST :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_REQUEST)

			u32  userindex;
		s32  otherindex;
		s64  othermemid;
	};
	//7070 申请加入xx队伍 队长同意
	struct S_TEAM_LEADER :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_LEADER)

			u32  userindex;
		s32  joinindex; //加入队伍玩家
		s64  joinmemid;
	};

	//7090 转让队长
	struct S_TEAM_NEWLEADER :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_NEWLEADER)

			u32  userindex;
		u8   mateindex;
	};

	//7100 队伍成员信息
	struct S_TEAM_INFO :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_INFO)
			u32  userindex;

		u32  other_index;//游戏索引
		s64  other_memid;//账号id
		s32  teamindex; //队伍索引
		u8   materindex; //成员索引
		u8   sex;//性别
		u8   job;//职业
		u16  level;//等级
		bool isleader;
		char nick[USER_MAX_NICK];//昵称
	};

	struct S_REBORN_ROLE : S_COMMAND_GATEBASE
	{
		this_constructor(S_REBORN_ROLE);

		u8	kind;				// 锁定类型
		s32 userindex;
		// S_VECTOR3 targetpos;		// 目标点
		inline void reset() { memset(this, 0, sizeof(S_REBORN_ROLE)); };
	};

	struct S_ROBOT_DATA : S_COMMAND_GATEBASE
	{
		this_constructor(S_ROBOT_DATA);
		u32 robotindex;
	};

	struct S_DROP_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_DROP_DATA)

		u32  userindex;
		u32  dropindex;
		s32  dropid;
	};
	//切换地图
	struct S_CHANGEMAP_BASE :S_COMMAND_GATEBASE
	{
		this_constructor(S_CHANGEMAP_BASE)

		u8   line;
		u32  userindex; //玩家唯一索引
		u32  mapid;     //需要切换到的地图ID

		inline void reset() { memset(this, 0, sizeof(S_CHANGEMAP_BASE)); }

	};

	struct  S_SKILL_3000 :public func::S_DATA_HEAD
	{
		u8 skillindex;
		u8 lock_type;
		s32 lock_index;
		S_VECTOR3   pos;
	};
	struct  S_SKILL_3100 :public func::S_DATA_HEAD
	{
		s32 userindex;
		s32 skillid;
		u8  level;
		u8 skillindex;
		u8  lock_type;//怪物索引
		s32 lock_index;
		S_VECTOR3   pos;
	};

#pragma pack(pop, packing)
}

#endif