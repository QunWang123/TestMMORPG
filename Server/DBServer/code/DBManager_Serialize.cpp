
#include "DBManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "ShareFunction.h"
#include <json.hpp>
#include "UserData.h"

using Json = nlohmann::json;

namespace db
{

	//1 �����������л�
 	void serializeJson(Json& js, app::S_ROLE_PROP_BASE* base)
 	{
 		js["id"] = base->id;
 		js["sourcefrom"] = base->sourcefrom;
 		js["type"] = base->type;
 		js["count"] = base->count;
 		js["createtime"] = base->createtime;
 		js["uniqueid"] = base->uniqueid;
 		js["money"] = base->money;

 		//std::string str = js.dump();
 		//printf("base...size:%d %s\n", str.length(),str.c_str());
 	}
	//flower5
	//2 װ���������л�
 	void serializeJson(Json& js, app::S_ROLE_PROP_EQUIP* equip)
 	{
 		//װ������
 		js["p_atk"] = equip->p_atk;
 		js["m_atk"] = equip->m_atk;
 		js["p_defend"] = equip->p_defend;
 		js["m_defend"] = equip->m_defend;
 		js["crit"] = equip->crit;
 		js["dodge"] = equip->dodge;
 		js["hp"] = equip->hp;
		js["mp"] = equip->mp;
 		js["nick"] = equip->nick;
 
 		//std::string str = js.dump();
 		//printf("equip...size:%d %s\n", str.length(), str.c_str());
 	}
	//flower5
	//3 ��ʯ�������л�
 	void serializeJson(Json& js, app::S_ROLE_PROP_GEM* gem)
 	{
 		//��ʯ����
 		js["gem_purple"] = gem->gem_purple;
 		js["gem_blue"] = gem->gem_blue;
 		js["gem_yellow"] = gem->gem_yellow;
 		js["gem_green"] = gem->gem_green;
 		js["gem_red"] = gem->gem_red;
 
 		//std::string str = js.dump();
 	}
	//flower5
	//�������л�Ϊ�ַ��� 
 	std::string serializeString(app::S_ROLE_STAND_BAG* bag)
 	{
 		Json arr = Json::array();
 		for (int i = 0; i < bag->num; i++)
 		{
 			if (i >= USER_MAX_BAG) continue;
 
 			app::S_ROLE_PROP* prop = &bag->bags[i];
 			if (prop->IsT() == false) continue;
 
 			//��ȡװ�����л��ַ���
 			Json js_base;
 			serializeJson(js_base, &prop->base);
 
 			//д����
 			Json js;
 			js["index"] = i;
 			js["base"] = js_base;
 
 			//�����װ�� ���������������
 			if (prop->base.type == app::EPT_EQUIP)
 			{
 				Json js_equip;
 				Json js_gem;
 				serializeJson(js_equip, &prop->equip);
 				serializeJson(js_gem, &prop->gem);
 				js["equip"] = js_equip;
 				js["gem"] = js_gem;
 			}
 			//���л�������
 			arr.emplace_back(js.dump());
 			// io::pushLog(io::EFT_RUN, "%s size:%d\n", js.dump(), arr.size());
 		}
 		std::string str = arr.dump();
 
 		// io::pushLog(io::EFT_RUN, "bag.................%d %d\n", arr.size(), str.size());
 
 		return str;
 	}
	//flower5
	//��ɫ������崩��װ�� ���л��ַ���
 	std::string serializeString(app::S_ROLE_STAND_COMBAT* combat)
 	{
 		Json arr = Json::array();
 		for (int i = 0; i < USER_MAX_EQUIP; i++)
 		{
			app::S_ROLE_PROP* prop = &combat->equip[i];
 			if (prop->IsT() == false) continue;
 
 			Json js_base;
 			Json js_equip;
 			Json js_gem;
 			serializeJson(js_base, &prop->base);
 			serializeJson(js_equip, &prop->equip);
 			serializeJson(js_gem, &prop->gem);
 
 			Json js;
 			js["index"] = i;
 			js["base"] = js_base;
 			js["equip"] = js_equip;
 			js["gem"] = js_gem;
 
 			//���л�������
 			arr.emplace_back(js.dump());
 			// printf("%s \n", arr.dump());
 		}
 		std::string str = arr.dump();
 
 		// io::pushLog(io::EFT_RUN, "bag.................%d %d\n", arr.size(), str.size());
 		// io::pushLog(io::EFT_RUN, "%s\n", str.c_str());
 
 		return str;
 	}

	//flower5
	//�ֿ����л�Ϊ�ַ��� 
// 	std::string serializeString(app::S_ROLE_STAND_WAREHOUSE* wh)
// 	{
// 		Json arr = Json::array();
// 		for (int i = 0; i < wh->num; i++)
// 		{
// 			if (i >= USER_MAX_WAREHOUSE) continue;
// 
// 			app::S_ROLE_PROP* prop = &wh->warehouses[i];
// 			if (prop->IsT() == false) continue;
// 
// 			//��ȡװ�����л��ַ���
// 			Json js_base;
// 			serializeJson(js_base, &prop->base);
// 
// 			//д����
// 			Json js;
// 			js["index"] = i;
// 			js["base"] = js_base;
// 
// 			//�����װ�� ���������������
// 			if (prop->base.type == EPT_EQUIP)
// 			{
// 				Json js_equip;
// 				Json js_gem;
// 				serializeJson(js_equip, &prop->equip);
// 				serializeJson(js_gem, &prop->gem);
// 				js["equip"] = js_equip;
// 				js["gem"] = js_gem;
// 			}
// 
// 
// 			//���л�������
// 			arr.emplace_back(js.dump());
// 			//io::pushLog(io::EFT_RUN, "%s size:%d\n", js.dump(), arr.size());
// 		}
// 		std::string str = arr.dump();
// 
// 		io::pushLog(io::EFT_RUN, "bag.................%d %d\n", arr.size(), str.size());
// 		//io::pushLog(io::EFT_RUN, "%s\n", str.c_str());
// 
// 		return str;
// 	}
	
}