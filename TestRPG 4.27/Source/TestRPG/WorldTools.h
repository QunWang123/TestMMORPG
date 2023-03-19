#pragma once
#include "UserData.h"

// ����תΪ�����
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

//����תΪ�������� ��-С
inline void posToGrid(FGRID_BASE* grid, FVector* pos, FVector* left)
{
	grid->row = (abs(pos->X - left->X) / (C_WORLDMAP_ONE_GRID));
	grid->col = (abs(pos->Y - left->Y) / (C_WORLDMAP_ONE_GRID));
}
//��������תΪ3D����
inline void gridToPos(FGRID_BASE * grid, FVector * pos, FVector * left)
{
	pos->X = left->X - (grid->row * C_WORLDMAP_ONE_GRID + 50);
	pos->Y = left->Y + grid->col * C_WORLDMAP_ONE_GRID + 50;
}