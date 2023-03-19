
#include "DBManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ShareFunction.h"
#include "json.hpp"
using Json = nlohmann::json;
using namespace app;
using namespace nlohmann::detail;

namespace db
{
	//******************************************************************************
	//flower5 
	//1 反序列化到2进制 基础数据 一定要验证json安全 防止崩溃
	void deserializeBinary(Json& js, app::S_ROLE_PROP_BASE* base)
	{
		if (js.is_object() == false) return;

		auto& id = js["id"];
		auto& sourcefrom = js["sourcefrom"];
		auto& type = js["type"];
		auto& count = js["count"];
		auto& createtime = js["createtime"];
		auto& uniqueid = js["uniqueid"];
		auto& money = js["money"];

		if (id.is_number())  base->id = id;
		if (sourcefrom.is_number())  base->sourcefrom = sourcefrom;
		if (type.is_number())  base->type = type;
		if (count.is_number())  base->count = count;
		if (createtime.is_number())  base->createtime = createtime;
		if (uniqueid.is_number())  base->uniqueid = uniqueid;
		if (money.is_number())  base->money = money;
	}
	//flower5 
    //2 反序列化到2进制 装备数据 一定要验证json安全 防止崩溃
	void deserializeBinary(Json& js, app::S_ROLE_PROP_EQUIP* equip)
	{
		if (js.is_object() == false) return;

		auto& p_atk = js["p_atk"];
		auto& m_atk = js["m_atk"];
		auto& p_defend = js["p_defend"];
		auto& m_defend = js["m_defend"];
		auto& crit = js["crit"];
		auto& dodge = js["dodge"];
		auto& hp = js["hp"];
		auto& mp = js["mp"];
		auto& nick = js["nick"];

		if (p_atk.is_number())  equip->p_atk = p_atk;
		if (m_atk.is_number())  equip->m_atk = m_atk;
		if (p_defend.is_number())  equip->p_defend = p_defend;
		if (m_defend.is_number())  equip->m_defend = m_defend;
		if (crit.is_number())  equip->crit = crit;
		if (dodge.is_number())  equip->dodge = dodge;
		if (hp.is_number())  equip->hp = hp;
		if (mp.is_number())  equip->mp = mp;
		if (nick.is_string())
		{
			std::string s_nick = nick;
			memcpy(equip->nick, s_nick.c_str(), USER_MAX_NICK);
		}
	}

	//flower5 
    //3 反序列化到2进制 装备数据 一定要验证json安全 防止崩溃
	void deserializeBinary(Json& js, app::S_ROLE_PROP_GEM* gem)
	{
		if (js.is_object() == false) return;

		auto& gem_purple = js["gem_purple"];
		auto& gem_blue = js["gem_blue"];
		auto& gem_yellow = js["gem_yellow"];
		auto& gem_green = js["gem_green"];
		auto& gem_red = js["gem_red"];

		if (gem_purple.is_number())  gem->gem_purple = gem_purple;
		if (gem_blue.is_number())  gem->gem_blue = gem_blue;
		if (gem_yellow.is_number())  gem->gem_yellow = gem_yellow;
		if (gem_green.is_number())  gem->gem_green = gem_green;
		if (gem_red.is_number())  gem->gem_red = gem_red;
	
	}

	//反序列化背包数组
 	void deserializeBinary_bag(std::string& value, app::S_ROLE_STAND_BAG* bag)
 	{
		// printf("all...size:%d %s\n", value.length(), value.c_str());
 		Json arr = Json::parse(value);
 		if (arr.is_array() == false) return;
 
 		for (auto it = arr.begin(); it != arr.end(); ++it)
 		{
 			std::string str = *it;
 			Json js = Json::parse(str);
 			if (js.is_object() == false) continue;
 
 			auto& js_index = js["index"];
 			if(js_index.is_number() == false) continue;
 			u32 index = js_index;
 			if (index >= USER_MAX_BAG) continue;
 			//1、设置道具基础数据
 			auto prop = &bag->bags[index];
 			deserializeBinary(js["base"], &prop->base);
 			if (prop->base.type != EPT_EQUIP) continue;
 
 			//2、如果是装备 设置装备数据
 			deserializeBinary(js["equip"], &prop->equip);
 			deserializeBinary(js["gem"], &prop->gem);
 		}
 	}

	//序列化战斗装备数组 角色属性上面穿戴的装备
	void deserializeBinary_combat(std::string& value, app::S_ROLE_STAND_COMBAT* combat)
	{
		Json arr = Json::parse(value);
		if (arr.is_array() == false) return;
	
		for (auto it = arr.begin(); it != arr.end(); ++it)
		{
			std::string str = *it;
			Json js = Json::parse(str);
			if (js.is_object() == false) continue;

			auto& js_index = js["index"];
			if (js_index.is_number() == false) continue;
	
			u32 index = js_index;
			if (index >= USER_MAX_EQUIP) continue;
	
			//1、设置道具基础数据
			auto prop = &combat->equip[index];
			deserializeBinary(js["base"], &prop->base);
			deserializeBinary(js["equip"], &prop->equip);
			deserializeBinary(js["gem"], &prop->gem);
		}
	}
	//反序列化仓库数组
// 	void deserializeBinary(std::string& value, app::S_ROLE_STAND_WAREHOUSE* wh)
// 	{
// 		Json arr = Json::parse(value);
// 		if (arr.is_array() == false) return;
// 
// 		for (auto it = arr.begin(); it != arr.end(); ++it)
// 		{
// 			std::string str = *it;
// 			Json js = Json::parse(str);
// 			if (js.is_object() == false) continue;
// 
// 			auto& js_index = js["index"];
// 			if (js_index.is_number() == false) continue;
// 			u32 index = js_index;
// 			if (index >= USER_MAX_WAREHOUSE) continue;
// 
// 			//1、设置道具基础数据
// 			auto prop = &wh->warehouses[index];
// 			deserializeBinary(js["base"], &prop->base);
// 			if (prop->base.type != EPT_EQUIP) continue;
// 
// 			//2、如果是装备 设置装备数据
// 			deserializeBinary(js["equip"], &prop->equip);
// 			deserializeBinary(js["gem"], &prop->gem);
// 		}
// 	}
	//json反序列化二进制
	void deserializeBinary(MySqlConnetor* mysql, app::S_USER_ROLE* role)
	{
		// role->reset();
		memset(role, 0, sizeof(app::S_USER_ROLE));
		//读取基础数据
		mysql->r("id", role->base.innate.id);
		mysql->r("job", role->base.innate.job);
		mysql->r("sex", role->base.innate.sex);
		mysql->r("nick", role->base.innate.nick);
		//经验
		mysql->r("level", role->base.exp.level);
		mysql->r("curexp", role->base.exp.currexp);
		//经济
		mysql->r("gold", role->base.econ.gold);
		mysql->r("diamonds", role->base.econ.diamonds);
		//状态
		mysql->r("face", role->base.status.face);
		mysql->r("mapid", role->base.status.mapid);
		mysql->r("c_mapid", role->base.status.c_mapid);

		//背包数量 仓库数量
 		mysql->r("bag_count", role->stand.bag.num);
// 		mysql->r("warehouse_gold", role->stand.warehouse.gold);
// 		mysql->r("warehouse_count", role->stand.warehouse.num);

		std::string pos;
		std::string c_pos;
		std::string skill;
		std::string atk;
		std::string bag;
		std::string combat;
		std::string warehouse;
		mysql->r("pos", pos);
		mysql->r("c_pos", c_pos);
		mysql->r("skill", skill);
		mysql->r("atk", atk);
		mysql->r("bag", bag);
		mysql->r("combat", combat);
		mysql->r("warehouse", warehouse);

		std::vector<std::string> arr = share::split(pos, ";", true);
		if (arr.size() == 3)
		{
			role->base.status.pos.x = atoi(arr[0].c_str());
			role->base.status.pos.y = atoi(arr[1].c_str());
			role->base.status.pos.z = atoi(arr[2].c_str());
		}
		arr.clear();
		arr = share::split(c_pos, ";", true);
		if (arr.size() == 3)
		{
			role->base.status.c_pos.x = atoi(arr[0].c_str());
			role->base.status.c_pos.y = atoi(arr[1].c_str());
			role->base.status.c_pos.z = atoi(arr[2].c_str());
		}

		//技能  1001,1,0;
		//1002,1,0;1003,1,0;1004,1,0;1005,1,0;1006,1,0;1007,1,0;1008,1,0;1009,1,0;1010,1,0
		arr.clear();
		arr = share::split(skill, ";", true);
		if (arr.size() == MAX_SKILL_COUNT)
		{
			for (int i = 0; i < MAX_SKILL_COUNT; i++)
			{
				std::vector<std::string> arr2 = share::split(arr[i], ",", true);
				if (arr2.size() == 3)
				{
					role->stand.myskill.skill[i].id				= atoi(arr2[0].c_str());
					role->stand.myskill.skill[i].level			= atoi(arr2[1].c_str());
					role->stand.myskill.skill[i].quick_index	= atoi(arr2[2].c_str());

// 					LOG_MESSAGE("mysql read skill...%d-%d \n",
// 						role->stand.myskill.skill[i].id, role->stand.myskill.skill[i].level);
				}
			}
		}
		//基础攻击
		arr.clear();
		arr = share::split(atk, ";", true);
		if (arr.size() == 10)
		{
			role->stand.atk.hp = atoi(arr[0].c_str());
			role->stand.atk.mp = atoi(arr[1].c_str());
			role->stand.atk.p_atk = atoi(arr[2].c_str());
			role->stand.atk.m_atk = atoi(arr[3].c_str());
			role->stand.atk.p_defend = atoi(arr[4].c_str());
			role->stand.atk.m_defend = atoi(arr[5].c_str());
			role->stand.atk.dodge = atoi(arr[6].c_str());
			role->stand.atk.crit = atoi(arr[7].c_str());
			role->stand.atk.dechp = atoi(arr[8].c_str());
			role->stand.atk.speed = atoi(arr[9].c_str());
		}
		//背包 战斗装备 仓库
		deserializeBinary_bag(bag, &role->stand.bag);
		deserializeBinary_combat(combat, &role->stand.combat);
		//deserializeBinary(warehouse, &role->stand.warehouse);
	}
}