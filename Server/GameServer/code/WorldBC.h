#include "WorldData.h"
#include "CommandData.h"
#include "CommandData2.h"
#include "RobotData.h"
#include "AppCopy.h"
#ifndef __APPWORLDBC_H
#define __APPWORLDBC_H


namespace app
{
	class TS_Broadcast
	{
	public:
		// 副本、组队
		//static void team_ChangeMap(S_USER_BASE* user, const u16 cmd);
		static void team_StartCopy(S_CMD_COPY& data, const u16 cmd);
		static void team_LeaveCopy(S_USER_BASE* user, s32 copyindex);
		static void bc_ResetCopy(S_COPY_ROOM* room, u16 childcmd);

		// 角色行为
		static void bc_RoleLeaveWorld(S_USER_BASE* user);
		static void bc_RoleState(S_USER_BASE* user);
		static void bc_RoleMove(S_USER_BASE* user, S_MOVE_ROLE* move);
		static void bc_RoleMoveLeaveBig(S_RECT_BASE* old, S_USER_BASE* user);
		static void bc_RoleSkill(S_USER_BASE* user, S_TEMP_SKILL_RUN_BASE* run); // 广播玩家开始技能
		static void bc_RoleBuff(S_USER_BASE* user, S_TEMP_BUFF_RUN_BASE* run); // 广播玩家buff
		static void bc_RoleSkillHP(S_USER_BASE* user, u8 hp_type, s32 hp_index, s32 atk, s32 curhp); // 广播玩家受到伤害
		static void bc_RoleDead(S_USER_BASE* user);		// 广播玩家死亡
		static void bc_RoleReborn(S_USER_BASE* user, S_VECTOR3* targetpos);		// 广播玩家死亡
		static void bc_RoleLevelUP(S_USER_BASE* user);		// 广播玩家升级
		// 经验
		static void do_SendExp(S_USER_BASE* user);
		// buff
		static void do_SendBuff(u16 cmd, s32 otherServerID, s32 otherUserID, u64 othermemID, u32 nodeindex, S_TEMP_BUFF_RUN_BASE* run);
		// atk
		static void do_SendAtk(S_USER_BASE* user_connect, S_USER_BASE* user);
		// 发送数值
		static void do_SendValue(u16 cmd, u32  index, s32 value, S_USER_BASE* user);
		// 发送金币
		static void do_SendGold(S_USER_BASE* user);
		// 发送背包
		static void do_SendUpdateProp(S_USER_BASE* user, u8 bagpos, bool iscount);
		static void do_SendCompat(S_USER_BASE* user, u8 bagpos, u8 equippos, u8 kind);
		static void do_SendSwap(S_USER_BASE* user, u8 equippos1, u8 equippos2);

		// robot
		static void bc_RobotState(S_ROBOT* _robot, u16 cmd, S_GRID_BASE* grid = NULL, s32 atk = 0);
		static void bc_RobotBuff(S_ROBOT* _robot, S_TEMP_BUFF_RUN_BASE* run); // 广播玩家buff
		static void bc_RobotSkillHP(S_ROBOT* _robot, u8 hp_type, s32 hp_index, s32 atk, s32 curhp); // 广播玩家受到伤害
		//掉落
		static void bc_DropInfo(u32 mapid, u32 dropindex);
		static void bc_DropDelete(u32 mapid, u32 dropindex);
		static void do_SendDropAdd(S_USER_BASE* user, void* _drop);
		static void do_SendDropDelete(S_USER_BASE* user, void* _drop);
		// 切图
		static void do_ChangeMap(S_USER_BASE* user, void* d, u32 size, s32 connectid, u16 cmd);
		// 通知队伍切图
		static void team_ChangeMap(S_USER_BASE* user, const u16 cmd);
		// DB服务器
		static void db_Disconnect(u32 userindex, u64 memid, u32 timelogin);		// 发送给DB服务器玩家掉线
		static void db_SaveStatusInfo(S_USER_BASE* user);						// 发送给DB服务器玩家状态
		static void db_ChangeMap(net::S_CLIENT_BASE* c, void* d, s32 userindex, u8 line, u32 mapid, u16 cmd, u16 childcmd);
		// 发送给DB服务器背包
		static void db_SendUpdateProp(S_USER_BASE* user, u8 bagpos, bool iscount);
		static void db_SaveLevelUP(S_USER_BASE* user);
		static void db_SaveCompat(S_USER_BASE* user, u8 bagpos, u8 equippos, u8 kind);
		static void db_SaveSwap(S_USER_BASE* user, u8 equippos1, u8 equippos2);
		static void db_SaveGold(S_USER_BASE* user);
		static void db_SendExp(S_USER_BASE* user);
	};
}




#endif // __TS_BROADCAST_H
