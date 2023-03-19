#include  "WorldTools.h"
#include "UserManager.h"
#include "WorldData.h"
#include "WorldMap.h"
#include "WorldBC.h"
#include "WorldScript.h"
#include "RobotManager.h"
#include <time.h>
using namespace script;
namespace app
{
	//�������� 
	bool TS_Tools::createProp(const s32 id, const u8 sourcefrom, const s32 count, const s64 rid, const char* nick, S_ROLE_PROP* prop)
	{
		u32 propindex = 0;
		u32 index = 0;
		SCRIPT_PROP* s_prop = script::findScript_Prop(id);
		if (s_prop == nullptr)
		{
			LOG_MESSAGE("WorldProp is error...%d-line:%d\n", id, __LINE__);
			return false;
		}

		switch (s_prop->type)
		{
		case EPT_EQUIP://װ��
		{
			bool iscreate = createEquip(id, sourcefrom, rid, nick, s_prop, prop);
			if (!iscreate)
			{
				LOG_MESSAGE("WorldProp is error...%d-line:%d\n", id, __LINE__);
				return false;
			}
		}
		break;
		case EPT_CONSUME://���������
		{
			bool iscreate = createConsume(id, sourcefrom, count, s_prop, prop);
			if (!iscreate)
			{
				LOG_MESSAGE("WorldProp is error...%d-line:%d\n", id, __LINE__);
				return false;
			}
		}
		break;
		case EPT_GOLD://���
			bool iscreate = createMoney(id, sourcefrom, s_prop, prop);
			if (!iscreate)
			{
				LOG_MESSAGE("WorldProp is error...%d-line:%d\n", id, __LINE__);
				return false;
			}
			break;
		}

		prop->base.money = s_prop->price;//���ۼ۸�
		prop->temp.script_prop = s_prop;
		return true;
	}


	//���װ������ֵ
	void randomEquipValue(S_ROLE_PROP* prop, SCRIPT_PROP_EQUIP* s_equip)
	{
		prop->equip.hp = s_equip->hp;
		prop->equip.mp = s_equip->mp;
		prop->equip.p_atk = s_equip->p_atk;
		prop->equip.p_defend = s_equip->p_defend;
		prop->equip.m_atk = s_equip->m_atk;
		prop->equip.m_defend = s_equip->m_defend;
		prop->equip.dodge = s_equip->dodge;
		prop->equip.crit = s_equip->crit;

		//����װ������ ���װ������
		switch (s_equip->kind)
		{
		case EPE_WEQPON:
		{
			prop->equip.p_atk	+= TS_Tools::Random(s_equip->r_atk) + 1;
			prop->equip.m_atk	+= TS_Tools::Random(s_equip->r_atk) + 1;
			prop->equip.crit	+= TS_Tools::Random(s_equip->r_crit) + 1;
			//�������80 �����һ������ ����ֵ
// 			u8 hp = TS_Tools::Random(100);
// 			if (hp > 80)
// 			{
// 				prop->equip.hp = TS_Tools::Random(s_equip->r_hp) + s_equip->level * 20;
// 			}
// 			//����
// 			u8 crit = TS_Tools::Random(100);
// 			if (hp > 90)
// 			{
// 				prop->equip.crit = TS_Tools::Random(s_equip->r_crit) + 1;
// 			}
		}
		break;
		case EPE_HEAD:
		{
			prop->equip.mp			+= TS_Tools::Random(s_equip->hp) + 1;
			prop->equip.m_defend	+= TS_Tools::Random(s_equip->r_defend) + 1;
		}
		break;
		case EPE_CLOTH:
		{
			prop->equip.hp			+= TS_Tools::Random(s_equip->r_hp) + 1;
			prop->equip.p_defend	+= TS_Tools::Random(s_equip->r_defend) + 1;
		}
		break;
		case EPE_DECORATIVE:
		{
			prop->equip.hp			+= TS_Tools::Random(s_equip->r_hp) + 1;
			prop->equip.mp			+= TS_Tools::Random(s_equip->r_hp) + 1;
			prop->equip.p_atk		+= TS_Tools::Random(s_equip->r_atk) + 1;
			prop->equip.p_defend	+= TS_Tools::Random(s_equip->r_defend) + 1;
			prop->equip.m_atk		+= TS_Tools::Random(s_equip->r_atk) + 1;
			prop->equip.m_defend	+= TS_Tools::Random(s_equip->r_defend) + 1;
			prop->equip.dodge		+= TS_Tools::Random(s_equip->r_dodge) + 1;
			prop->equip.crit		+= TS_Tools::Random(s_equip->r_crit) + 1;
		}
		break;
		case EPE_SHOES:
		{
			prop->equip.hp			+= TS_Tools::Random(s_equip->r_hp) + 1;
			prop->equip.mp			+= TS_Tools::Random(s_equip->r_hp) + 1;
			prop->equip.dodge		+= TS_Tools::Random(s_equip->r_dodge) + 1;
		}
		break;
		case EPE_NECKLACE:
		{
			prop->equip.hp			+= TS_Tools::Random(s_equip->r_hp) + 1;
			prop->equip.mp			+= TS_Tools::Random(s_equip->r_hp) + 1;
			prop->equip.p_atk		+= TS_Tools::Random(s_equip->r_atk) + 1;
			prop->equip.m_atk		+= TS_Tools::Random(s_equip->r_atk) + 1;
		}
		break;
		case EPE_RING:
		case EPE_BANGLE:
		{
			prop->equip.hp			+= TS_Tools::Random(s_equip->r_hp) + 1;
			prop->equip.mp			+= TS_Tools::Random(s_equip->r_hp) + 1;
			prop->equip.p_atk		+= TS_Tools::Random(s_equip->r_atk) + 1;
			prop->equip.p_defend	+= TS_Tools::Random(s_equip->r_defend) + 1;
			prop->equip.m_atk		+= TS_Tools::Random(s_equip->r_atk) + 1;
			prop->equip.m_defend	+= TS_Tools::Random(s_equip->r_defend) + 1;
			prop->equip.dodge		+= TS_Tools::Random(s_equip->r_dodge) + 1;
			prop->equip.crit		+= TS_Tools::Random(s_equip->r_crit) + 1;
		}
		break;
		}
	}
	bool TS_Tools::createEquip(const s32 id, const u8 sourcefrom, const s64 rid, const char* nick, void* script, S_ROLE_PROP* prop)
	{
		SCRIPT_PROP* s_prop = (SCRIPT_PROP*)script;
		if (s_prop == nullptr) return false;
		if (s_prop->equip.id < 100000) return false;

		prop->reset();

		prop->base.id = id;
		prop->base.type = EPT_EQUIP;
		prop->base.sourcefrom = sourcefrom;
		prop->base.createtime = time(NULL);
		prop->base.uniqueid = time(NULL);
		prop->base.count = 1;//���ܺϲ�
		prop->base.money = s_prop->price;//���ۼ۸�

		//��Ҵ���װ��
		if (sourcefrom == EPF_PLAYERFORGE)
		{
			if (nick == nullptr) return false;
			memcpy(prop->equip.nick, nick, USER_MAX_NICK);
		}

		randomEquipValue(prop, &s_prop->equip);
		return true;
	}
	bool TS_Tools::createConsume(const s32 id, const u8 sourcefrom, const s32 count, void* script, S_ROLE_PROP* prop)
	{
		SCRIPT_PROP* s_prop = (SCRIPT_PROP*)script;
		if (s_prop == nullptr) return false;
		if (s_prop->consume.id < 100000) return false;

		prop->reset();
		prop->base.id = id;
		prop->base.type = EPT_CONSUME;
		prop->base.sourcefrom = sourcefrom;
		prop->base.createtime = time(NULL);
		prop->base.uniqueid = time(NULL);
		prop->base.money = s_prop->price;//���ۼ۸�
		prop->base.count = count;
		if (prop->base.count < 1) prop->base.count = 1;
		else if (prop->base.count > s_prop->maxcount) prop->base.count = s_prop->maxcount;

		return true;
	}
	//������� ֻ�����ڵ����ڵ���
	bool  TS_Tools::createMoney(const s32 id, const u8 sourcefrom, void* script, S_ROLE_PROP* prop)
	{
		SCRIPT_PROP* s_prop = (SCRIPT_PROP*)script;
		if (s_prop == nullptr) return false;
		prop->reset();
		prop->base.id = id;
		prop->base.type = EPT_GOLD;
		prop->base.sourcefrom = sourcefrom;
		prop->base.createtime = time(NULL);
		prop->base.uniqueid = time(NULL);
		prop->base.count = 1;
		prop->base.money = TS_Tools::Random(s_prop->price) + (s_prop->price >> 1);//���ۼ۸�

		return true;
	}
}