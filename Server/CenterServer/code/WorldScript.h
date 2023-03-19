#ifndef  ____WORLDSCRIPT_H
#define  ____WORLDSCRIPT_H
#include "UserData.h"

namespace script
{
	

	//9����ͼ�ű�
	struct SCRIPT_MAP_BASE
	{
		bool    isreborn;
		u32     mapid;
		u8      maptype;
		s16     row;//����������x
		s16     col;//����������y
		u8      copy_player;//���������������
		u8      copy_level;//�ȼ�Ҫ��
		u8      copy_difficulty;//�Ѷ�
		u8      copy_win;//ʤ������
		u32     copy_limittime;//����ʱ��
	};


	extern void initScript();
	extern SCRIPT_MAP_BASE* findScript_Map(s32 id);
	extern bool isCopyMap(s32 mapid);
}


#endif