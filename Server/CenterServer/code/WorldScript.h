#ifndef  ____WORLDSCRIPT_H
#define  ____WORLDSCRIPT_H
#include "UserData.h"

namespace script
{
	

	//9、地图脚本
	struct SCRIPT_MAP_BASE
	{
		bool    isreborn;
		u32     mapid;
		u8      maptype;
		s16     row;//出生点坐标x
		s16     col;//出生点坐标y
		u8      copy_player;//副本最大人数上限
		u8      copy_level;//等级要求
		u8      copy_difficulty;//难度
		u8      copy_win;//胜利条件
		u32     copy_limittime;//限制时间
	};


	extern void initScript();
	extern SCRIPT_MAP_BASE* findScript_Map(s32 id);
	extern bool isCopyMap(s32 mapid);
}


#endif