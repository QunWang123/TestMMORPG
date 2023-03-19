// Fill out your copyright notice in the Description page of Project Settings.


#include "UserData.h"
#include "MyActor.h"
#include "WorldScript.h"
#include <io.h>
#include "Misc/Paths.h"

int32 delaytime;
uint8 loginkey[USER_MAX_KEY];
FUSER_MEMBER_ROLE rolebase[USER_MAX_ROLE];
FUSER_ROLE_BASE		__myUserData;
FMAP_DATA __CurMapInfo;
TMap<int32, FOTHER_ROLE_BASE> onLineDatas;
char FileExePath[MAX_EXE_LEN];


void InitMapInfo(int mapid)
{
// 	__CurMapInfo.leftpos.X = 910;
// 	__CurMapInfo.leftpos.Y = -920;
// 	__CurMapInfo.leftpos.Z = 138;
// 	__CurMapInfo.row = 60;
// 	__CurMapInfo.col = 60;
	__CurMapInfo.mapID = mapid;

	char fname[1000];
	char filename[200];
	sprintf_s(filename, "%d.bin", __CurMapInfo.mapID);
	memset(fname, 0, 1000);

	sprintf_s(fname, "%s//bin//%s", FileExePath, filename);
	int isexist = _access(fname, 0);
	if (isexist == -1) return;

	int m_row = 0;
	int m_col = 0;
	FS_VECTOR leftpos;

	FILE * m_File = _fsopen(fname, "rb+", _SH_DENYNO);
	if (m_File == nullptr) return;
	fseek(m_File, 0, SEEK_SET);
	fread(&leftpos, 1, 12, m_File);
	// 便宜到12 读取最大值
	fseek(m_File, 12, SEEK_SET);
	fread(&m_row, 1, 4, m_File);
	// 便宜到16 读取最大值
	fseek(m_File, 16, SEEK_SET);
	fread(&m_col, 1, 4, m_File);

	if (m_row >= MAX_ROW || m_row < 0) return;
	if (m_col >= MAX_COL || m_col < 0) return;

	__CurMapInfo.leftpos.X = leftpos.x;
	__CurMapInfo.leftpos.Y = leftpos.y;
	__CurMapInfo.leftpos.Z = leftpos.z;
	__CurMapInfo.row = m_row;
	__CurMapInfo.col = m_col;
}




