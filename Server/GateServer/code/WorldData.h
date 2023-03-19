#ifndef __WORLDDATA_H
#define __WORLDDATA_H
#include "INetBase.h"
#include "UserData.h"

namespace app
{
#pragma pack(push, packing)
#pragma pack(1)
	// 网关上的用户数据
	struct S_USER_GATE
	{
		u8			state;							// 当前状态
		u8			line;							// 当前所在线路
		u64			memid;							// 当前登录的游戏账号
		u32			mapid;							// 当前所在地图id
		s32			userindex;						// 游戏服务器上哈希数组的索引
		u16			level;
		u32			temp_RoleTime;					// 申请登录游戏获取角色数据时间
		u32			temp_MoveSpeed;					// 移动速度
		u32			temp_GetCompatTime;				// 获取战斗装备时间 
		u32			temp_GetBagTime;				// 获取背包时间 
// 		u32			temp_GetWareHouseTime;			// 获取仓库时间 
// 		u32			temp_GetTaskTime;				// 获取任务时间 
		u32			temp_GetChangeMapTime;			// 获取切换地图时间
		u32			temp_GetCopyTime;				// 获取副本操作时间
		char		name[USER_MAX_MEMBER];
		char		nick[USER_MAX_NICK];

		net::ITCPClient*		tcpGame;			// 连接游戏服务器的通信指针
		net::S_CLIENT_BASE*		connection;			// 记录的玩家连接数据(就是engine里的那个客户端)

		inline void reset()
		{
			state = 0;
			line = 0;
			memid = 0;
			mapid = 0;
			userindex = -1;
			temp_RoleTime = 0;
			temp_MoveSpeed = 0;
			temp_GetCompatTime = 0;
			temp_GetBagTime = 0;
			// temp_GetWareHouseTime = 0;
			// temp_GetTaskTime = 0;
			temp_GetChangeMapTime = 0;
			temp_GetCopyTime = 0;
			memset(name, 0, USER_MAX_MEMBER);
			memset(nick, 0, USER_MAX_NICK);

			tcpGame = nullptr;
			connection = nullptr;
		}
	};
#pragma pack(pop, packing)

}



#endif // __WORLDDATA_H
