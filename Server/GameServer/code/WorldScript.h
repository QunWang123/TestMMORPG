#ifndef  ____WORLDSCRIPT_H
#define  ____WORLDSCRIPT_H

#include "UserData.h"

#define MAX_SKILLSCRIPT_COUNT		20000
#define MAX_BUFFSCRIPT_COUNT		100
#define MAX_MONSTERSCRIPT_COUNT		30000
#define MAX_LEVELSCRIPT_COUNT		1000
#define MAX_TASKSCRIPT_COUNT		30000

namespace script
{
	struct SCRIPT_SKILL_BASE 
	{
		this_constructor(SCRIPT_SKILL_BASE);

		s32		id;				// ����ID
		s32		level;			// ���ܵȼ�
		s32		maxlevel;		// ����ܵȼ�
		s32		type;			// 1.���� 2.������Ŀ�� 3.����
		s32		islock;			// �Ƿ���Ҫ����  0����	1��Ҫ
		s32		isfollow;		// �Ƿ��������ƶ�
		s32		needjob;		// ��Ҫְҵ
		s32		needlevel;		// ��Ҫ��ҽ�ɫ�ȼ�
		s32		needmp;			// ��Ҫħ��ֵ
		s32		needhp;			// ��Ҫ����ֵ
		s32		coldtime;		// ��ȴʱ��
		s32		distance;		// ��������
		s32		radius;			// �뾶 cmΪ��λ
		s32		damagehp;		// �˺�ֵ
		s32		factorhp;		// �˺�ϵ��
		s32		runingtime;		// ����ʱ��
		s32		flytime;		// ���ܷ���ʱ��
		s32		waittime;		// �����ü���һ���˺�
		s32		continued;		// �Ƿ�����������Լ��ܣ�
		s32		damagecount;	// �˺�����
		s32		buff_id;		// �����ɵ�buff_id
		s32		buff_percent;	// ����buff�İٷֱ�
	};

	struct SCRIPT_BUFF_BASE
	{
		this_constructor(SCRIPT_BUFF_BASE);

		s32		id;				// buff_id 
		s32		state;			// ״̬
		s32		type;			// 0ʹ����Ʒ�����ģ�1ʹ�ü��ܲ����ģ�2����
		s32		runningtime;
		// ����
		s32		add_hp;			// ���� �ٷֱ�
		s32		add_defend;		// ���� �ٷֱ�
		s32		add_atk;		// ���� �ٷֱ�
		// ����
		s32		subtract_hp;	// ��Ѫ
	};

	//4����ҵȼ� ���Ա�
	struct  SCRIPT_PALYER_LEVEL
	{
		s32   level;
		s32   exp;
		s32   hp;
		s32   mp;
		s32   p_atk;
		s32   m_atk;
		s32   p_defend;
		s32   m_defend;
		s32   hit;  //����
		s32   dodge;//����
		s32   crit; //����
		s32   dechp;//�˺�����
		s32   speed;//�ƶ��ٶ�

	};

	struct SCRIPT_DROP_ID
	{
		this_constructor(SCRIPT_DROP_ID);
		s32  id;//����ID
		s32  probability;//��ֱ�
		s32  count;//��������
	};

	struct SCRIPT_MONSTER_BASE
	{
		this_constructor(SCRIPT_MONSTER_BASE);
		u32 id;
		u32 level;
		u32 maxhp;
		u32 maxmp;
		// u32 model_id;
		u32 atk;
		u32 def;
		u32 dodge;
		u32 crit;
		u32 movespeed; 
		u32 exp; 
		u32 time_death; 
		u32 time_fresh; 
		u8  range;			// Ѳ�߷�Χ
		u8  alert;			// ���䷶Χ
		u8  isatk; 
		u8  isboss; 

		std::vector<s32> skill_ids;
		std::vector<SCRIPT_DROP_ID*> drop_ids;
	};

	//6��װ����
	struct  SCRIPT_PROP_EQUIP
	{
		s32   id;   //Ψһ���
		s32   kind; //����
		u8   level;//װ���ȼ�
		u8   color;//Ʒ��
		u8   job;  //ְҵ
		u8   part; //װ��λ��

		s32   hp;//����
		s32   mp;//ħ��
		s32   p_atk;//������
		s32   m_atk;//ħ������
		s32   p_defend;//�������
		s32   m_defend;//ħ������
		s32   dodge;//����
		s32   crit; //����

		s32   r_atk;
		s32   r_defend;
		s32   r_dodge;
		s32   r_crit;
		s32   r_hp;

		s32   needgold;    //����װ����Ҫ���
		s32   needpropid_1;//����װ����Ҫ����ID1
		s32   needpropnum_1;//��Ҫ����1
		s32   needpropid_2;
		s32   needpropnum_2;

		inline void reset()
		{
			memset(this, 0, sizeof(SCRIPT_PROP_EQUIP));
		}
	};
	//7�����ĵ��߱�
	struct  SCRIPT_PROP_CONSUME
	{
		s32   id;   //Ψһ���
		s32   kind; //����
		u16   maxcount;//����������

		s32   potion_value; //ҩƷ��ֵ
		s32   potion_cdtime;//ҩƷʱ��

		u8   gem_class;//���Է���
		s32  gem_value;//��ʯ��ֵ

		inline void reset()
		{
			memset(this, 0, sizeof(SCRIPT_PROP_CONSUME));
		}
	};

	//5�������ܱ�
	struct  SCRIPT_PROP
	{
		s32  id;      //Ψһ���
		u8   type;    //����
		s32  kind;    //����
		u8   color;   //Ʒ��
		u8   minlevel;//ʹ����С�ȼ�
		s32  maxcount;//��������
		s32  price;   //���ۼ۸�
		u8   maxuse;       //�Ƿ�����ʹ��
		u8   issplit;      //�Ƿ�ɲ��
		u8   isdestroy;    //�Ƿ������
		u8   istransaction;//�Ƿ�ɽ���
		u8   isshow;       //�Ƿ��չʾ
		u8   isbind;       //�Ƿ��
		u8   issell;       //�Ƿ�ɳ���
		u8   tab;//����ҳǩ

		SCRIPT_PROP_EQUIP    equip;
		SCRIPT_PROP_CONSUME  consume;
	};


	//9����ͼ�ű�
	struct SCRIPT_MAP_BASE
	{
		this_constructor(SCRIPT_MAP_BASE);
		bool    isreborn;
		u32     mapid;
		u8      maptype;
		s16     row;				//	����������x
		s16     col;				//	����������y
		u8      copy_player;		//	���������������
		u8      copy_level;			//	�ȼ�Ҫ��
		u8      copy_difficulty;	//	�Ѷ�
		u8      copy_win;			//	ʤ������
		u32     copy_limittime;		//	����ʱ��
	};

	extern void initScript();
	extern SCRIPT_SKILL_BASE* findScript_Skill(s32 id, s32 level);
	extern SCRIPT_BUFF_BASE* findScript_Buff(s32 id);
	extern SCRIPT_PALYER_LEVEL* findScript_Level(s32 level);
	extern SCRIPT_MONSTER_BASE* findScript_Monster(s32 id);
	extern SCRIPT_PROP* findScript_Prop(s32 id);
	extern SCRIPT_MAP_BASE* findScript_Map(s32 id);
}


#endif