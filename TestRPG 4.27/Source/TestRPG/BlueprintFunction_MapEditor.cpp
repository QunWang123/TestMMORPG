#include "BlueprintFunction.h"
#include "AppManager.h"
#include "CommandData.h"
#include "MyActor.h"
#include "UserData.h"
#include "WorldTools.h"



// 设置左上角坐标
void UBlueprintFunction::editor_SetMaxRowCol(FVector left, FVector right)
{
	__LeftPos = left;
	__MapBin.left_pos.x = left.X;
	__MapBin.left_pos.y = left.Y;
	__MapBin.left_pos.z = left.Z;

	__MapBin.row = abs(right.X - __LeftPos.X) / C_WORLDMAP_ONE_GRID;
	__MapBin.col = abs(right.Y - __LeftPos.Y) / C_WORLDMAP_ONE_GRID;
}

// 获取新的坐标 校准坐标，将1m*1m格子中得位置转到正中间，左手坐标系，x向上
FVector UBlueprintFunction::editor_UpdateNewPosition(FVector pos)
{
	FVector newPos = pos;
	FGRID_BASE grid;
	posToGrid(&grid, &pos, &__LeftPos);
	gridToPos(&grid, &newPos, &__LeftPos);


	//FString ss = FString::Printf(TEXT("g...%d/%d  %.2f/%.2f"), grid.row,grid.col,newPos.X,newPos.Y);
	//if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 5, FColor::Emerald, ss);
	return newPos;
}

//坐标转为网格
FGRID_BASE UBlueprintFunction::editor_PosToGrid(FVector pos)
{
	FGRID_BASE grid;
	posToGrid(&grid, &pos, &__LeftPos);
	return grid;
}
//网格转为坐标
FVector UBlueprintFunction::editor_GridToPos(FGRID_BASE grid)
{
	FVector pos;
	gridToPos(&grid, &pos, &__LeftPos);
	return pos;
}

void UBlueprintFunction::editor_SetCurState(int32 state)
{
	__EditorState = state;
}
int32 UBlueprintFunction::editor_getCurState()
{
	return __EditorState;
}

//插入actor
void UBlueprintFunction::editor_InsertActor(AMyActor* a)
{
	int key = a->Row * 100000 + a->Col;
	__MyActor.insert(std::make_pair(key, a));

	//添加数据
	insertBinValue(a->Row, a->Col, a->ID);
}
//删除actor
void UBlueprintFunction::editor_DeleteActor(int row, int col)
{
	int key = row * 100000 + col;
	auto it = __MyActor.find(key);
	if (it != __MyActor.end())
	{
		auto d = it->second;
		__MyActor.erase(it);
		d->Destroy();
	}
	//清理 初始化数据
	deleteBinValue(row, col);
}
//判断是不是存在actor
int32 UBlueprintFunction::editor_IsExistActor(int row, int col)
{
	int key = row * 100000 + col;
	auto it = __MyActor.find(key);
	if (it != __MyActor.end()) return it->second->ID;
	return 0;
}

void UBlueprintFunction::editor_DeleteAllSpecID(int id)
{
	deleteBinValue(id);
}


//销毁 初始化数据
void UBlueprintFunction::editor_DestoryAll()
{
	__EditorState = 0;

	auto it = __MyActor.begin();
	while (it != __MyActor.end())
	{
		auto d = it->second;
		it = __MyActor.erase(it);
		d->Destroy();
	}

	__MyActor.clear();
	__MapBin.reset();
}

//保存文件
void UBlueprintFunction::editor_SaveName(FString filename)
{
	FString s = filename + ".bin";
	TCHAR* pSendData = s.GetCharArray().GetData();
	int nDataLen = FCString::Strlen(pSendData);
	char* dst = (char*)TCHAR_TO_ANSI(pSendData);

	writeBinData(dst);
}
//打开文件
void UBlueprintFunction::editor_OpenName(FString filename)
{
	FString s = filename + ".bin";
	TCHAR* pSendData = s.GetCharArray().GetData();
	int nDataLen = FCString::Strlen(pSendData);
	char* dst = (char*)TCHAR_TO_ANSI(pSendData);

	readBinData(dst);
}

TArray<FString> UBlueprintFunction::editor_GetMapData()
{
	TArray<FString> data;
	for (int row = 0; row < __MapBin.row; row++)
	{
		for (int col = 0; col < __MapBin.col; col++)
		{
			int value = __MapBin.value[row][col];
			if (value == 0) continue;

			
			FString ss = FString::Printf(TEXT("%d,%d,%d"),row,col,value);
			//if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 10.0f, FColor::Emerald, ss);
			data.Add(ss);

		}
	}

	return data;
}

