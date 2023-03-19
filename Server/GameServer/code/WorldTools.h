#ifndef __WORLDTOOLS_H
#define __WORLDTOOLS_H

#include "WorldData.h"

namespace app
{
	struct S_DIR
	{
		s8 x;
		s8 y;
	};


	class TS_Tools
	{
	public:
		static s32 getDistance(S_VECTOR3* pos1, S_VECTOR3* pos2);
		static s32 getDistance(S_GRID_BASE* pos1, S_GRID_BASE* pos2);
		static f64 getDistance_f64(S_VECTOR3* pos1, S_VECTOR3* pos2);
		static void posToGrid(S_GRID_BASE* grid, S_VECTOR3* pos, S_VECTOR3* left);
		static void posToGridMax(S_GRID_BASE* grid, S_VECTOR3* pos, S_VECTOR3* left);
		static void gridToMax(S_GRID_BASE* grid, S_GRID_BASE* big);
		static void gridToPos(S_GRID_BASE* grid, S_VECTOR3* pos, S_VECTOR3* left);

		static s32 computeAngle(S_VECTOR3* a, S_VECTOR3* b, S_VECTOR3* c);
		static bool computeDir(S_GRID_BASE* apos, S_GRID_BASE* bpos, u8& dir);		// 用来计算机器人和玩家之间的方向，以便机器人追逐

		// 锁定的目标是否在user范围内
		static bool isInRadius(S_USER_BASE* user, u8 type, void* enemy, S_VECTOR3* targetpos, void* sk);
		static bool isInRadius(void* _robot, u8 type, void* enemy, S_VECTOR3* targetpos, void* sk);
		static void setSkillDamage(S_USER_BASE* user, u8 type, void* enemy, S_TEMP_SKILL_RUN_BASE* run, void* sk);
		static void setSkillDamage(void* _robot, u8 type, void* enemy, S_TEMP_SKILL_RUN_BASE* run, void* sk);

		static f32 getPointToLine(S_VECTOR3* p1, S_VECTOR3* p2);
		static f32 getPointToLine(S_VECTOR3* p1, S_VECTOR3* p2, S_VECTOR3* point, f32 value);

		static s32 getPointToLine_3D(S_VECTOR3* p1, S_VECTOR3* p2);
		static s32 getPointToLine_3D(S_VECTOR3* p1, S_VECTOR3* p2, S_VECTOR3* point, s32 value);

		static s32 Random(s32 value);
		static s32 RandomRange(u32 min, u32 max);

		// 计算周围一圈圈的点的坐标，中心点为（0，0）。这样求世界地图中（x,y）的周围坐标时，遍历加即可
		static void calculateRectanglePoints();
		static bool findEmptyPosNoSprite(const u32 mapid, S_GRID_BASE* src, s32 layer, bool isdrop = false);
		static void* findSprite(S_RECT_BASE* edge, const u8 type, const u32 index, const s32 layer, const u32 mapid, S_GRID_BASE* grid = NULL);
		
		// buff
		static void buff_skill_create(void* sprite, void* sk, const u8 type);
		static void buff_runing(void* sprite, const u8 type, const s32 value);
		static void buff_skill_random(void* sprite, void* sk, const u8 type);

		//道具生成
		static  bool  createProp(const s32 id, const u8 sourcefrom, const s32 count, const s64 rid, const char* nick, S_ROLE_PROP* prop);
		static  bool  createEquip(const s32 id, const u8 sourcefrom, const s64 rid, const char* nick, void* script, S_ROLE_PROP* prop);
		static  bool  createConsume(const s32 id, const u8 sourcefrom, const s32 count, void* script, S_ROLE_PROP* prop);
		static  bool  createMoney(const s32 id, const u8 sourcefrom, void* script, S_ROLE_PROP* prop);
	};
	extern S_DIR  __Dirs[MAX_DIR];
}

#endif

