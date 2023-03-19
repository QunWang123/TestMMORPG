#include "WorldTools.h"
#include "WorldScript.h"
#include "WorldMap.h"
#include "UserManager.h"
#include "RobotManager.h"

namespace app
{
	//1、直角坐标系中，xy均为正的是第一象限，然后逆时针依次为二三四 
	//-----------------y轴++-----------
	//-------(-1,1)----|-------(1,1)第一象限---
	//-----------------|-------------
	//------(-1,0)---(0,0)-----(1,0)-----x轴++
	//-----------------|---------------
	//------(-1,-1)----|-------(1,-1)第四象限--
	//2、角度范围
	//---------135-90--45-------
	//---------180-x---0--------
	//---------225-270-315------

	//-----7-----0-----1----------
	//-----6-----X-----2----------
	//-----5-----4-----3----------
	//----------------------------
		//计算方向的角度
	f32 Pi = 3.1415926;
	//f32 CG_Range = 22.5f;
	f32 CG_sRange[9] = { 112.5f,67.5f,22.5f,337.5f,292.5f,247.5f,202.5f,157.5f,112.5f };



	//---------7-0-1----------
	//---------6-x-2----------
	//---------5-4-3----------
	S_DIR  __Dirs[MAX_DIR] =
	{ { 0,-1 }, //0
	{ 1,-1 },   //1
	{ 1,  0 },  //2
	{ 1, 1 },   //3
	{ 0, 1 },   //4
	{ -1,1 },   //5
	{ -1, 0 },  //6
	{ -1,-1 } };//7

	//计算周围空闲点
	u32 C_A_RECTANGLE_0 = 1;
	u32 C_A_RECTANGLE_1 = C_A_RECTANGLE_0 + 8 * 1;		// 9
	u32 C_A_RECTANGLE_2 = C_A_RECTANGLE_1 + 8 * 2;		// 25
	u32 C_A_RECTANGLE_3 = C_A_RECTANGLE_2 + 8 * 3;		// 49
	u32 C_A_RECTANGLE_4 = C_A_RECTANGLE_3 + 8 * 4;		// 81
	u32 C_A_RECTANGLE_5 = C_A_RECTANGLE_4 + 8 * 5;		// 121
	u32 C_A_RECTANGLE_6 = C_A_RECTANGLE_5 + 8 * 6;		// 169
	u32 C_A_RECTANGLE_7 = C_A_RECTANGLE_6 + 8 * 7;		// 225
	u32 C_A_RECTANGLE_8 = C_A_RECTANGLE_7 + 8 * 8;		// 289	
	u32 C_A_RECTANGLE_9 = C_A_RECTANGLE_8 + 8 * 9;		// 361
	u32 C_A_RECTANGLE_10 = C_A_RECTANGLE_9 + 8 * 10;	// 441
	u32 C_A_RECTANGLE_MAX = C_A_RECTANGLE_10;			// 441
	S_GRID_BASE  C_A_RECTANGLEs[441];

	//计算点数据
	void TS_Tools::calculateRectanglePoints()
	{
		u32  pos = 0;
		C_A_RECTANGLEs[pos].row = 0;
		C_A_RECTANGLEs[pos].col = 0;
		pos = 1;
		for (u32 c = 1; c < C_A_RECTANGLE_MAX; c++)
		{
			for (u8 dir = 0; dir < MAX_DIR; dir++)
			{
				for (u32 x = 0; x < c; x++)
				{
					switch (dir)
					{
					case 0:
					case 7:
						C_A_RECTANGLEs[pos].col = __Dirs[dir].x * c + x;
						C_A_RECTANGLEs[pos].row = __Dirs[dir].y * c;
						break;
					case 1:
					case 2:
						C_A_RECTANGLEs[pos].col = __Dirs[dir].x * c;
						C_A_RECTANGLEs[pos].row = __Dirs[dir].y * c + x;
						break;
					case 3:
					case 4:
						C_A_RECTANGLEs[pos].col = __Dirs[dir].x * c - x;
						C_A_RECTANGLEs[pos].row = __Dirs[dir].y * c;
						break;
					case 5:
					case 6:
						C_A_RECTANGLEs[pos].col = __Dirs[dir].x * c;
						C_A_RECTANGLEs[pos].row = __Dirs[dir].y * c - x;
						break;
					}
					pos++;
					if (pos >= C_A_RECTANGLE_MAX) return;
				}
			}
		}
	}

	s32 TS_Tools::getDistance(S_VECTOR3* pos1, S_VECTOR3* pos2)
	{
		s32 distance = (s32)sqrt((pos1->x - pos2->x) * (pos1->x - pos2->x) + (pos1->y - pos2->y) * (pos1->y - pos2->y) + (pos1->z - pos2->z) * (pos1->z - pos2->z));
		return distance;
	}

	s32 TS_Tools::getDistance(S_GRID_BASE* pos1, S_GRID_BASE* pos2)
	{
		u32 len = sqrt((pos1->row - pos2->row) * (pos1->row - pos2->row) + (pos1->col - pos2->col) * (pos1->col - pos2->col));
		return len;
	}

	f64 TS_Tools::getDistance_f64(S_VECTOR3* pos1, S_VECTOR3* pos2)
	{
		f64 distance = sqrt(((f64)pos1->x - pos2->x) * ((f64)pos1->x - pos2->x) + ((f64)pos1->y - pos2->y) * ((f64)pos1->y - pos2->y) + ((f64)pos1->z - pos2->z) * ((f64)pos1->z - pos2->z));
		return distance;
	}

	void TS_Tools::posToGrid(S_GRID_BASE* grid, S_VECTOR3* pos, S_VECTOR3* left)
	{
		grid->row = abs(pos->x - left->x) / C_WORLDMAP_ONE_GRID;
		grid->col = abs(pos->y - left->y) / C_WORLDMAP_ONE_GRID;
	}

	// 坐标转为大格子
	void TS_Tools::posToGridMax(S_GRID_BASE* grid, S_VECTOR3* pos, S_VECTOR3* left)
	{
		grid->row = abs(pos->x - left->x) / (C_WORLDMAP_ONE_GRID * C_WORLDMAP_NODE_X);
		grid->col = abs(pos->y - left->y) / (C_WORLDMAP_ONE_GRID * C_WORLDMAP_NODE_Y);

	}

	void TS_Tools::gridToPos(S_GRID_BASE* grid, S_VECTOR3* pos, S_VECTOR3* left)
	{
		pos->x = left->x - (grid->row * C_WORLDMAP_ONE_GRID + 50);
		pos->y = left->y + grid->col * C_WORLDMAP_ONE_GRID + 50;
		// pos->z = left->z;
	}

	// 格子坐标转为大格子坐标
	void TS_Tools::gridToMax(S_GRID_BASE* grid, S_GRID_BASE* big)
	{
		big->row = grid->row / C_WORLDMAP_NODE_X;
		big->col = grid->col / C_WORLDMAP_NODE_Y;
	}

	s32 TS_Tools::computeAngle(S_VECTOR3* a, S_VECTOR3* b, S_VECTOR3* c)
	{
		f64 ab_length = getDistance_f64(a, b);
		f64 ac_length = getDistance_f64(a, c);
		f64 bc_length = getDistance_f64(b, c);
		f64 value = (ab_length * ab_length + ac_length * ac_length - bc_length * bc_length) / (2 * ab_length * ac_length);

		s32 angle = acos(value) * 180 / Pi;
		return angle;
	}

	//p1 使用技能的主体玩家的坐标 p2目标点 形成直线
	f32 TS_Tools::getPointToLine(S_VECTOR3* p1, S_VECTOR3* p2)
	{
		s32 x1 = p1->y;
		s32 x2 = p2->y;
		s32 y1 = p1->x;
		s32 y2 = p2->x;

		f32 a = y2 - y1;
		f32 b = x1 - x2;
		return (sqrt(a * a + b * b));
	}
	//p1 使用技能的主体玩家的坐标 p2目标点 point为检查坐标点 value 被除数 getPointToLine(p1,p2):
	f32 TS_Tools::getPointToLine(S_VECTOR3* p1, S_VECTOR3* p2, S_VECTOR3* point, f32 value)
	{
		if (value == 0) return 0;

		s32 x1 = p1->y;
		s32 x2 = p2->y;
		s32 y1 = p1->x;
		s32 y2 = p2->x;
		s32 x0 = point->y;
		s32 y0 = point->x;

// 		f32 a = y2 - y1;
// 		f32 b = x1 - x2;
// 		f32 c = x2 * y1 - x1 * y2;

		f32 line = fabs((y2 - y1) * x0 + (x1 - x2) * y0 + ((x2 * y1) - (x1 * y2)));

		return line / value;
	}


	s32 TS_Tools::getPointToLine_3D(S_VECTOR3* p1, S_VECTOR3* p2)
	{
		s32 x1 = p1->y;
		s32 x2 = p2->y;
		s32 y1 = p1->x;
		s32 y2 = p2->x;
		s32 z1 = p1->z;
		s32 z2 = p2->z;

		s32 a = y2 - y1;
		s32 b = x1 - x2;
		s32 c = z2 - z1;
		return (a * a + b * b + c * c);
	}

	s32 TS_Tools::getPointToLine_3D(S_VECTOR3* p1, S_VECTOR3* p2, S_VECTOR3* point, s32 value)
	{
		s32 x1 = p1->x;
		s32 x2 = p2->x;
		s32 x0 = point->x;

		s32 y1 = p1->y;
		s32 y2 = p2->y;
		s32 y0 = point->y;

		s32 z1 = p1->z;
		s32 z2 = p2->z;
		s32 z0 = point->z;
		
		f32 TempValue = (-1) * ((x1 - x0) * (x2 - x1) + (y1 - y0) * (y2 - y1) + (z1 - z0) * (z2 - z1)) / value;

		S_VECTOR3 TempVector;
		TempVector.x = TempValue * (x2 - x1) + x1;
		TempVector.y = TempValue * (y2 - y1) + y1;
		TempVector.z = TempValue * (z2 - z1) + z1;

		s32 distance = getDistance(point, &TempVector);
		return distance;
	}

	s32 TS_Tools::Random(s32 value)
	{
		if (value <= 0) return 0;
		return rand() % value;
	}

	bool TS_Tools::isInRadius(S_USER_BASE* user, u8 type, void* enemy, S_VECTOR3* targetpos, void* sk)
	{
		// if (user->node.type == type && user->node.index == index) return false;
		if (enemy == NULL) return false;
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == nullptr) return false;

		script::SCRIPT_SKILL_BASE* skillscript = (script::SCRIPT_SKILL_BASE*)sk;

		s32 check_distance = skillscript->distance;
		s32 atk_distance = 0;
		S_VECTOR3 enemy_pos;
		bool canCal = false;

		switch (type)
		{
		case N_ROLE:
		{
			S_USER_BASE* enemy_user = (S_USER_BASE*)enemy;
			if (enemy_user)
			{
				if (user->role.base.status.mapid != enemy_user->role.base.status.mapid) return false;
				enemy_pos = enemy_user->role.base.status.pos;
				canCal = true;
			}
			
		}
			break;
		case N_MONSTER:
		case N_PET:
		case N_NPC:
		{
			S_ROBOT* enemy_robot = (S_ROBOT*)enemy;
			if (enemy_robot)
			{
				if (user->role.base.status.mapid != enemy_robot->data.status.mapid) return false;
				gridToPos(&enemy_robot->bc.grid_pos, &enemy_pos, &map->leftpos);
				// 这里并不好，因为这个z坐标
				enemy_pos.z = user->role.base.status.pos.z;
				canCal = true;
			}
		}
			break;
		default:
			return false;
		}

		if (canCal)
		{
 			S_ROLE_BASE_STATUS* stat = &user->role.base.status;

			switch (skillscript->type)
			{
			case E_SKILL_USE_TYPE_HALF:
			{
				s32 angle = computeAngle(&stat->pos, targetpos, &enemy_pos);
				LOG_MESSAGE("skill enemy angle %d \n", angle);
				if (angle > 80) return false;
			}
				break;
			case E_SKILL_USE_TYPE_TARGET:
			{
				s32 hp_distance = TS_Tools::getDistance(targetpos, &enemy_pos);
				if (hp_distance > skillscript->radius) return false;

				//攻击距离检查,这是最远距离了(即点到圆心+半径)
				check_distance = skillscript->distance + skillscript->radius;
			}
				break;
			case E_SKILL_USE_TYPE_SELF:
				break;
			}
			//检查攻击距离
			atk_distance = TS_Tools::getDistance(&stat->pos, &enemy_pos);
			return atk_distance <= check_distance;
		}
		return false;
	}

	bool TS_Tools::isInRadius(void* _robot, u8 type, void* enemy, S_VECTOR3* targetpos, void* sk)
	{
		// if (user->node.type == type && user->node.index == index) return false;
		if (_robot == NULL || sk == NULL || enemy == NULL) return false;
		S_ROBOT* robot = (S_ROBOT*)_robot;
		if (robot == NULL || !robot->isLive()) return false;

		S_WORLD_MAP* map = __World->getMap(robot->data.status.mapid);
		if (map == nullptr) return false;

		script::SCRIPT_SKILL_BASE* skillscript = (script::SCRIPT_SKILL_BASE*)sk;

		s32 check_distance = skillscript->distance;
		s32 atk_distance = 0;
		S_VECTOR3 enemy_pos;
		bool canCal = false;

		switch (type)
		{
		case N_ROLE:
		{
			S_USER_BASE* enemy_user = (S_USER_BASE*)enemy;
			if (enemy_user)
			{
				if (robot->data.status.mapid != enemy_user->role.base.status.mapid) return false;
				enemy_pos = enemy_user->role.base.status.pos;
				canCal = true;
			}
		}
		break;
		case N_MONSTER:
		case N_PET:
		case N_NPC:
		{
			S_ROBOT* enemy_robot = (S_ROBOT*)enemy;
			if (enemy_robot)
			{
				if (robot->data.status.mapid != enemy_robot->data.status.mapid) return false;
				gridToPos(&enemy_robot->bc.grid_pos, &enemy_pos, &map->leftpos);
				// 这里并不好，因为这个z坐标
				// enemy_pos.z = user->role.base.status.pos.z;
				canCal = true;
			}
		}
		break;
		default:
			return false;
		}

		if (canCal)
		{
			S_VECTOR3 self_pos;
			TS_Tools::gridToPos(&robot->bc.grid_pos, &self_pos, &map->leftpos);

			S_ROBOT_STATUS* stat = &robot->data.status;

			switch (skillscript->type)
			{
			case E_SKILL_USE_TYPE_HALF:
			{
				s32 angle = computeAngle(&self_pos, targetpos, &enemy_pos);
				LOG_MESSAGE("skill enemy angle %d \n", angle);
				if (angle > 80) return false;
			}
			break;
			case E_SKILL_USE_TYPE_TARGET:
			{
// 				s32 angle = computeAngle(&self_pos, targetpos, &enemy_pos);
// 				LOG_MESSAGE("skill enemy angle %d \n", angle);
// 				if (angle > 80) return false;

				s32 hp_distance = TS_Tools::getDistance(targetpos, &enemy_pos);
				if (hp_distance > skillscript->radius) return false;

				//攻击距离检查,这是最远距离了(即点到圆心+半径)
				check_distance = skillscript->distance + skillscript->radius;
			}
			break;
			case E_SKILL_USE_TYPE_SELF:
				break;
			}
			//检查攻击距离
			atk_distance = TS_Tools::getDistance(&self_pos, &enemy_pos);
			return atk_distance <= check_distance;
		}
		return false;
	}

	s32 TS_Tools::RandomRange(u32 min, u32 max)
	{
		s32 num = max - min;
		if (num <= 0) return 0;
		return  rand() % (num + 1) + min;
	}

	bool TS_Tools::findEmptyPosNoSprite(const u32 mapid, S_GRID_BASE* src, s32 layer, bool isdrop /*= false*/)
	{
		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return false;

		u32 pos = 0;
		S_GRID_BASE save;
		save = src;

		while (pos < C_A_RECTANGLE_MAX)
		{
			src->row = C_A_RECTANGLEs[pos].row + save.row;
			src->col = C_A_RECTANGLEs[pos].col + save.col;
			if (map->IsSpriteCollide_NoSprite(src, layer, isdrop))
			{
				return true;
			}
			pos++;
		}

		LOG_MESSAGE("WorldTools FindEmptyPosNoSprite  is error..  inline:%d\n", __LINE__);
		return false;
	}

	void* TS_Tools::findSprite(S_RECT_BASE* edge, const u8 type, const u32 index, const s32 layer, const u32 mapid, S_GRID_BASE* grid /*= NULL*/)
	{
		switch (type)
		{
		case N_ROLE:
		{
			auto lock_user = __UserManager->findUser(edge, index, layer, mapid);
			if (lock_user == NULL) return NULL;
			if (grid != NULL) *grid = lock_user->bc.grid_pos;
			return lock_user;
		}
		case N_NPC:
		case N_MONSTER:
		case N_PET:
		{
			auto lock_robot = __RobotManager->findRobot(edge, index, layer, mapid);
			if (lock_robot == NULL) return NULL;
			if (grid != NULL)  *grid = lock_robot->bc.grid_pos;
			return lock_robot;
		}
		}
		return NULL;
	}

	//计算方向
	bool TS_Tools::computeDir(S_GRID_BASE* apos, S_GRID_BASE* bpos, u8& dir)
	{
		if (apos->row == bpos->row && apos->col == bpos->col) return false;

		f32  x = bpos->col - apos->col;
		f32  y = bpos->row - apos->row;
		f32 distance = sqrt(x * x + y * y);
		x = x / distance;
		//反三角函数 反余玄函数计算出来是弧度值 = acos(x)
		//弧度公式 = 角度angle * (Pi/180);
		//acos(x) = angle * (Pi/180);
		//angle = acos(x) * (180 / pi)

		//在 1 2象限 x 算出来的角度是 0-180度
		//在3 4象限 算出来的 角度 也是0-180度
		//y < 0 说明 目标位置 在原位置的上方
		//y > 0 在下半位置算出来的角度是0-180 所以需要 360-
		f32 angle = acos(x) * (180.0f / Pi);
		if (y > 0) angle = 360 - angle;


		for (u8 i = 0; i < 8; i++)
		{
			if (angle >= CG_sRange[i + 1] && angle <= CG_sRange[i])
			{
				dir = i;
				return true;
			}
			if (angle >= 0 && angle <= CG_sRange[2])
			{
				dir = 2;
				return true;
			}
			if (angle >= 337.5f && angle <= 360)
			{
				dir = 2;
				return true;
			}
		}

		return false;
	}
}


