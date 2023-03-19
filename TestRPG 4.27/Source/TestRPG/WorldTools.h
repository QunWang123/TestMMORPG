#pragma once
#include "UserData.h"

// 坐标转为大格子
inline void posToGridMax(FGRID_BASE* grid, FVector* pos, FVector* left)
{
	grid->row = abs(pos->X - left->X) / (C_WORLDMAP_ONE_GRID * C_WORLDMAP_NODE_X);
	grid->col = abs(pos->Y - left->Y) / (C_WORLDMAP_ONE_GRID * C_WORLDMAP_NODE_Y);

}

inline bool isNineRect(FGRID_BASE* grid, FGRID_BASE* target)
{
	if (target->row >= grid->row - 1 && target->row <= grid->row + 1 &&
		target->col >= grid->col - 1 && target->col <= grid->col + 1) return true;
	return false;
}

//坐标转为格子坐标 大-小
inline void posToGrid(FGRID_BASE* grid, FVector* pos, FVector* left)
{
	grid->row = (abs(pos->X - left->X) / (C_WORLDMAP_ONE_GRID));
	grid->col = (abs(pos->Y - left->Y) / (C_WORLDMAP_ONE_GRID));
}
//网格坐标转为3D坐标
inline void gridToPos(FGRID_BASE * grid, FVector * pos, FVector * left)
{
	pos->X = left->X - (grid->row * C_WORLDMAP_ONE_GRID + 50);
	pos->Y = left->Y + grid->col * C_WORLDMAP_ONE_GRID + 50;
}