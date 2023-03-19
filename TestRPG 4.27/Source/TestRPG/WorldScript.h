#ifndef  ____WORLDSCRIPT_H
#define  ____WORLDSCRIPT_H

#include "UserData.h"
#include <vector>

#define MAX_SKILLSCRIPT_COUNT		20000
#define MAX_BUFFSCRIPT_COUNT		100
#define MAX_MONSTERSCRIPT_COUNT		30000
#define MAX_MAP_COUNT				50

namespace script
{
	struct SCRIPT_SKILL_BASE 
	{
		// this_constructor(SCRIPT_SKILL_BASE);
		SCRIPT_SKILL_BASE()
		{
			memset(this, 0, sizeof(SCRIPT_SKILL_BASE));
		}

		int32		id;				// ����ID
		int32		level;			// ���ܵȼ�
		int32		maxlevel;		// ����ܵȼ�
		int32		type;			// 1.���� 2.������Ŀ�� 3.����
		int32		islock;			// �Ƿ���Ҫ����  0����	1��Ҫ
		int32		isfollow;		// �Ƿ��������ƶ�
		int32		needjob;		// ��Ҫְҵ
		int32		needlevel;		// ��Ҫ��ҽ�ɫ�ȼ�
		int32		needmp;			// ��Ҫħ��ֵ
		int32		needhp;			// ��Ҫ����ֵ
		int32		coldtime;		// ��ȴʱ��
		int32		distance;		// ��������
		int32		radius;			// �뾶 cmΪ��λ
		int32		damagehp;		// �˺�ֵ
		int32		factorhp;		// �˺�ϵ��
		int32		runingtime;		// ����ʱ��
		int32		flytime;		// ���ܷ���ʱ��
		int32		waittime;		// �����ü���һ���˺�
		int32		continued;		// �Ƿ�����������Լ��ܣ�
		int32		damagecount;	// �˺�����
		int32		buff_id;		// �����ɵ�buff_id
		int32		buff_percent;	// ����buff�İٷֱ�
		FString		name;
	};

	struct SCRIPT_BUFF_BASE
	{
		int32		id;				// buff_id 
		int32		state;			// ״̬
		int32		type;			// 0ʹ����Ʒ�����ģ�1ʹ�ü��ܲ����ģ�2����
		int32		runningtime;
		// ����
		int32		add_hp;			// ���� �ٷֱ�
		int32		add_defend;		// ���� �ٷֱ�
		int32		add_atk;		// ���� �ٷֱ�
		// ����
		int32		subtract_hp;	// ��Ѫ
		FString		name;
	};

	struct SCRIPT_DROP_ID
	{
		int32  id;//����ID
		int32  probability;//��ֱ�
		int32  count;//��������
	};

	struct SCRIPT_MONSTER_BASE
	{
		int32 id;
		int32 level;
		int32 maxhp;
		int32 maxmp;
		int32 model_id;
		int32 atk;
		int32 def;
		int32 dodge;			// ����
		int32 crit;				// ����
		int32 movespeed;
		int32 exp;
		int32 time_death;
		int32 time_fresh;
		uint8  range;			// Ѳ�߷�Χ
		uint8  alert;			// ���䷶Χ
		uint8  isatk;
		uint8  isboss;

		FString nick;
		std::vector<int32> skill_ids;
		std::vector<SCRIPT_DROP_ID*> drop_ids;
	};

	//9����ͼ�ű�
	struct SCRIPT_MAP_BASE
	{
		int32     isreborn;
		int32    mapid;
		uint8    maptype;
		int16    row;				//	����������x
		int16    col;				//	����������y
		uint8    copy_player;		//	���������������
		uint8    copy_level;		//	�ȼ�Ҫ��
		uint8    copy_difficulty;	//	�Ѷ�
		uint8    copy_win;			//	ʤ������
		int32    copy_limittime;	//	����ʱ��

		FString mapname;
	};

	extern void initScript();
	extern SCRIPT_SKILL_BASE* fingScript_Skill(int32 id, int32 level);
	extern SCRIPT_BUFF_BASE* fingScript_Buff(int32 id);
	extern SCRIPT_MONSTER_BASE* findScript_Monster(int32 id);
	extern SCRIPT_MAP_BASE* findScript_Map(int32 id);
}



#endif