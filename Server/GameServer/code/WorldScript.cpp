#include "WorldScript.h"
#include "../../share/ShareFunction.h"
#include <map>
#include "UserData.h"

namespace script
{
	SCRIPT_SKILL_BASE*	__ScriptSkills[MAX_SKILLSCRIPT_COUNT];
	SCRIPT_BUFF_BASE*	__ScriptBuffs[MAX_BUFFSCRIPT_COUNT];
	SCRIPT_PALYER_LEVEL* __ScriptLevels[MAX_LEVELSCRIPT_COUNT];
	SCRIPT_MONSTER_BASE*	__ScriptMonsters[MAX_MONSTERSCRIPT_COUNT];
	SCRIPT_MAP_BASE* __ScriptMaps[MAX_MAP_ID];

	std::map<s32, SCRIPT_PROP_EQUIP*> __ScriptEquips;
	std::map<s32, SCRIPT_PROP_CONSUME*> __ScriptConsumes;
	std::map<s32, SCRIPT_PROP*> __ScriptProps;

	SCRIPT_SKILL_BASE* findScript_Skill(s32 id, s32 level)
	{
		if (id <= 0 || level <= 0) return nullptr;
		u32 index = id * 4 + level * 100;
		if (index >= MAX_SKILLSCRIPT_COUNT) return nullptr;
		return __ScriptSkills[index];
	}

	SCRIPT_BUFF_BASE* findScript_Buff(s32 id)
	{
		if (id < 0 || id >= MAX_BUFFSCRIPT_COUNT)return nullptr;
		return __ScriptBuffs[id];
	}

	SCRIPT_PALYER_LEVEL* findScript_Level(s32 level)
	{
		if (level >= MAX_LEVELSCRIPT_COUNT) return nullptr;
		return __ScriptLevels[level];
	}

	SCRIPT_MONSTER_BASE* findScript_Monster(s32 id)
	{
		if (id < 0 || id >= MAX_MONSTERSCRIPT_COUNT)return nullptr;
		return __ScriptMonsters[id];
	}

	SCRIPT_PROP_EQUIP* findScript_Equip(s32 id)
	{
		auto it = __ScriptEquips.find(id);
		if (it != __ScriptEquips.end())
		{
			return it->second;
		}
		return nullptr;
	}

	SCRIPT_PROP_CONSUME* findScript_Consume(s32 id)
	{
		auto it = __ScriptConsumes.find(id);
		if (it != __ScriptConsumes.end())
		{
			return it->second;
		}
		return nullptr;
	}

	//查找道具
	SCRIPT_PROP* findScript_Prop(s32 id)
	{
		auto it = __ScriptProps.find(id);
		if (it == __ScriptProps.end()) return nullptr;

		SCRIPT_PROP* prop = it->second;
		return prop;
	}

	SCRIPT_MAP_BASE* findScript_Map(s32 id)
	{
		if (id < 0 || id >= MAX_MAP_ID) return nullptr;
		return __ScriptMaps[id];
	}

	void loadScript_Skill()
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath, "%scsv\\%s", func::FileExePath, "skill.csv");

		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			LOG_MESSAGE("skill.csv is error... \n");
			return;
		}

		bool isfirstblv = false;

		// 逐行读取
		while (fgets(lineblv, 1000, fpblv))
		{
			// 第一行都是些变量名称什么的，跳过
			if (!isfirstblv)
			{
				isfirstblv = true;
				continue;
			}
			std::string countstr = lineblv;
			size_t n = countstr.find_last_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(n + 1, countstr.size() - n);
			}

			n = countstr.find_first_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(0, n);
			}

			std::vector<std::string> str = share::split(countstr, ",", true);
			if (str.size() == 23)
			{
				SCRIPT_SKILL_BASE* skill = new SCRIPT_SKILL_BASE();

				skill->id = atoi(str[0].c_str());
				skill->level = atoi(str[1].c_str());
				skill->maxlevel = atoi(str[2].c_str());
				skill->type = atoi(str[3].c_str());
				skill->isfollow = atoi(str[4].c_str());
				skill->needjob = atoi(str[5].c_str());
				skill->needmp = atoi(str[6].c_str());
				skill->needhp = atoi(str[7].c_str());
				skill->needlevel = atoi(str[8].c_str());
				skill->coldtime = atoi(str[9].c_str());
				skill->distance = atoi(str[10].c_str());
				skill->radius = atoi(str[11].c_str());
				skill->damagehp = atoi(str[12].c_str());
				skill->factorhp = atoi(str[13].c_str());
				skill->flytime = atoi(str[14].c_str());
				skill->waittime = atoi(str[15].c_str());
				skill->continued = atoi(str[16].c_str());
				skill->damagecount = atoi(str[17].c_str());
				skill->runingtime = atoi(str[18].c_str());
				skill->islock = atoi(str[19].c_str());
				skill->buff_id = atoi(str[20].c_str());
				skill->buff_percent = atoi(str[21].c_str());

				u32 index = skill->id * 4 + skill->level * 100;
				if (index >= MAX_SKILLSCRIPT_COUNT)
				{
					LOG_MESSAGE("skill err... %d-%d-%d-%d \n", index, skill->id, skill->level, skill->type);
					delete skill;
					continue;
				}
				// LOG_MESSAGE("skill... %d-%d-%d-%d \n", index, skill->id, skill->level, skill->type);
				__ScriptSkills[index] = skill;
			}
		}
	}

	void loadScript_Buff()
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath, "%scsv\\%s", func::FileExePath, "buff.csv");

		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			LOG_MESSAGE("buff.csv is error... \n");
			return;
		}

		bool isfirstblv = false;

		// 逐行读取
		while (fgets(lineblv, 1000, fpblv))
		{
			// 第一行都是些变量名称什么的，跳过
			if (!isfirstblv)
			{
				isfirstblv = true;
				continue;
			}
			std::string countstr = lineblv;
			size_t n = countstr.find_last_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(n + 1, countstr.size() - n);
			}

			n = countstr.find_first_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(0, n);
			}

			std::vector<std::string> str = share::split(countstr, ",", true);
			if (str.size() == 9)
			{
				SCRIPT_BUFF_BASE* buff = new SCRIPT_BUFF_BASE();

				buff->id = atoi(str[0].c_str());
				buff->state = atoi(str[1].c_str());
				buff->type = atoi(str[2].c_str());
				buff->runningtime = atoi(str[3].c_str());
				buff->add_hp = atoi(str[4].c_str());
				buff->add_defend = atoi(str[5].c_str());
				buff->add_atk = atoi(str[6].c_str());
				buff->subtract_hp = atoi(str[7].c_str());
				// buff->needlevel = atoi(str[8].c_str());
				

				// u32 index = skill->id * 4 + skill->level * 100;
				if (buff->id >= MAX_BUFFSCRIPT_COUNT)
				{
					LOG_MESSAGE("skill err... %d-%d \n", buff->id, buff->state);
					delete buff;
					continue;
				}
				LOG_MESSAGE("skill... %d-%d \n", buff->id, buff->state);
				__ScriptBuffs[buff->id] = buff;
			}
		}
	}

	void loadScript_Monster()
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath, "%scsv\\%s", func::FileExePath, "monster.csv");

		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			LOG_MESSAGE("monster.csv is error... \n");
			return;
		}

		bool isfirstblv = false;

		// 逐行读取
		while (fgets(lineblv, 1000, fpblv))
		{
			// 第一行都是些变量名称什么的，跳过
			if (!isfirstblv)
			{
				isfirstblv = true;
				continue;
			}

			std::string countstr = lineblv;
			size_t n = countstr.find_last_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(n + 1, countstr.size() - n);
			}

			n = countstr.find_first_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(0, n);
			}

			std::vector<std::string> str = share::split(countstr, ",", true);
			if (str.size() == 20)
			{
				SCRIPT_MONSTER_BASE* monster = new SCRIPT_MONSTER_BASE();
				monster->id = atoi(str[0].c_str());
				monster->level = atoi(str[1].c_str());
				monster->maxhp = atoi(str[2].c_str());
				monster->maxmp = atoi(str[3].c_str());
				monster->atk = atoi(str[4].c_str());
				monster->def = atoi(str[5].c_str());
				monster->dodge = atoi(str[6].c_str());
				monster->crit = atoi(str[7].c_str());
				monster->movespeed = atoi(str[8].c_str());
				monster->exp = atoi(str[9].c_str());
				monster->time_death = atoi(str[10].c_str());
				monster->time_fresh = atoi(str[11].c_str());
				monster->range = atoi(str[12].c_str());
				monster->alert = atoi(str[13].c_str());
				monster->isatk = atoi(str[14].c_str());
				monster->isboss = atoi(str[15].c_str());
				monster->drop_ids.clear();
				std::vector<std::string> tmpdropstr = share::split(str[16].c_str(), ";", true);
				for (std::string t : tmpdropstr)
				{
					std::vector<std::string> arr = share::split(t.c_str(), "|", true);
					if (arr.size() != 3) continue;
					SCRIPT_DROP_ID* did = new SCRIPT_DROP_ID();
					did->id = atoi(arr[0].c_str());
					did->probability = atoi(arr[1].c_str());
					did->count = atoi(arr[2].c_str());
					monster->drop_ids.emplace_back(did);
				}

				std::vector<std::string> tmpskillstr = share::split(str[17].c_str(), ";", true);
				if (tmpskillstr.size() > MAX_SKILL_COUNT)
				{
					delete monster;
					continue;
				}
				monster->skill_ids.clear();
				for (std::string t : tmpskillstr)
				{
					monster->skill_ids.push_back(atoi(t.c_str()));
				}

				// monster->model_id = atoi(str[18].c_str());
				if (monster->id >= MAX_MONSTERSCRIPT_COUNT || monster->skill_ids.size() < 1)
				{
					LOG_MESSAGE("load monsterscript err...\n");
					delete monster;
					continue;
				}
				__ScriptMonsters[monster->id] = monster;
			}
		}
	}

	void loadScript_Level()
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath, "%scsv\\%s", func::FileExePath, "level.csv");

		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];

		if (fpblv == nullptr)
		{
			LOG_MESSAGE("loadScript_Level.csv iserror... \n");
			return;
		}

		bool isfirstblv = false;
		//1、读取skill表
		while (fgets(lineblv, 1000, fpblv))
		{
			if (!isfirstblv)
			{
				isfirstblv = true;
				continue;
			}
			std::string countstr = lineblv;
			size_t n = countstr.find_last_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(n + 1, countstr.size() - n);
			}

			n = countstr.find_first_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(0, n);
			}

			SCRIPT_PALYER_LEVEL* level = new SCRIPT_PALYER_LEVEL();
			std::vector<std::string> str = share::split(countstr, ",", true);
			if (str.size() == 0) continue;

			level->level = atol(str[0].c_str());
			level->exp = atol(str[1].c_str());
			level->hp = atol(str[2].c_str());
			level->mp = atol(str[3].c_str());
			level->p_atk = atol(str[4].c_str());
			level->m_atk = atol(str[5].c_str());
			level->p_defend = atol(str[6].c_str());
			level->m_defend = atol(str[7].c_str());
			level->hit = atol(str[8].c_str());
			level->dodge = atol(str[9].c_str());
			level->crit = atol(str[10].c_str());
			level->dechp = atol(str[11].c_str());
			level->speed = atol(str[12].c_str());

			if (level->level >= MAX_LEVELSCRIPT_COUNT)
			{
				delete level;
				continue;
			}

			__ScriptLevels[level->level] = level;
		}
	}

	//加载装备配置表
	void loadScript_Equip()
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath, "%scsv\\%s", func::FileExePath, "equip.csv");
		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			LOG_MESSAGE("loadScript_Equip.csv iserror... \n");
			return;
		}
		bool isfirstblv = false;

		//1、读取monster表
		while (fgets(lineblv, 1000, fpblv))
		{
			if (!isfirstblv)
			{
				isfirstblv = true;
				continue;
			}
			std::string countstr = lineblv;
			size_t n = countstr.find_last_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(n + 1, countstr.size() - n);
			}

			n = countstr.find_first_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(0, n);
			}

			SCRIPT_PROP_EQUIP* equip = new SCRIPT_PROP_EQUIP();
			std::vector<std::string> str = share::split(countstr, ",", true);
			if (str.size() == 0) continue;

			equip->id = atol(str[0].c_str());
			equip->kind = atol(str[1].c_str());
			equip->level = atol(str[2].c_str());
			equip->color = atol(str[3].c_str());
			equip->job = atol(str[4].c_str());
			equip->part = atol(str[5].c_str());
			equip->hp = atol(str[6].c_str());
			equip->mp = atol(str[7].c_str());
			equip->p_atk = atol(str[8].c_str());
			equip->m_atk = atol(str[9].c_str());
			equip->p_defend = atol(str[10].c_str());
			equip->m_defend = atol(str[11].c_str());
			equip->dodge = atol(str[12].c_str());
			equip->crit = atol(str[13].c_str());
			equip->r_atk = atol(str[14].c_str());
			equip->r_defend = atol(str[15].c_str());
			equip->r_dodge = atol(str[16].c_str());
			equip->r_crit = atol(str[17].c_str());
			equip->r_hp = atol(str[18].c_str());


			std::vector<std::string> props = share::split(str[20].c_str(), ";", true);
			if (props.size() != 3) continue;

			equip->needgold = atol(props[0].c_str());

			std::vector<std::string> props_2 = share::split(props[1].c_str(), "-", true);
			if (props_2.size() == 2)
			{
				equip->needpropid_1 = atol(props_2[0].c_str());
				equip->needpropnum_1 = atol(props_2[1].c_str());
			}


			std::vector<std::string> props_3 = share::split(props[2].c_str(), "-", true);
			if (props_3.size() == 2)
			{
				equip->needpropid_2 = atol(props_3[0].c_str());
				equip->needpropnum_2 = atol(props_3[1].c_str());
			}

			__ScriptEquips.insert(std::make_pair(equip->id, equip));
		}
	}
	//加载消耗类道具表
	void loadScript_Consume()
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath, "%scsv\\%s", func::FileExePath, "consume.csv");
		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			LOG_MESSAGE("loadScript_Level.csv iserror... \n");
			return;
		}
		bool isfirstblv = false;

		//1、读取monster表
		while (fgets(lineblv, 1000, fpblv))
		{
			if (!isfirstblv)
			{
				isfirstblv = true;
				continue;
			}
			std::string countstr = lineblv;
			size_t n = countstr.find_last_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(n + 1, countstr.size() - n);
			}

			n = countstr.find_first_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(0, n);
			}

			SCRIPT_PROP_CONSUME* consume = new SCRIPT_PROP_CONSUME();
			std::vector<std::string> str = share::split(countstr, ",", true);
			if (str.size() == 0) continue;

			consume->id = atol(str[0].c_str());
			consume->kind = atol(str[1].c_str());
			consume->potion_value = atol(str[2].c_str());
			consume->potion_cdtime = atol(str[3].c_str());
			consume->gem_class = atol(str[4].c_str());
			consume->gem_value = atol(str[5].c_str());

			__ScriptConsumes.insert(std::make_pair(consume->id, consume));
		}
	}

	//加载道具总表
	void loadScript_Prop()
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath, "%scsv\\%s", func::FileExePath, "prop.csv");
		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			LOG_MESSAGE("loadScript_Prop.csv iserror... \n");
			return;
		}
		bool isfirstblv = false;

		//1、读取prop表
		while (fgets(lineblv, 1000, fpblv))
		{
			if (!isfirstblv)
			{
				isfirstblv = true;
				continue;
			}
			std::string countstr = lineblv;
			size_t n = countstr.find_last_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(n + 1, countstr.size() - n);
			}

			n = countstr.find_first_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(0, n);
			}

			SCRIPT_PROP* prop = new SCRIPT_PROP();
			std::vector<std::string> str = share::split(countstr, ",", true);
			if (str.size() == 0) continue;

			prop->id = atol(str[0].c_str());
			prop->type = atol(str[1].c_str());
			prop->kind = atol(str[2].c_str());
			prop->color = atol(str[3].c_str());
			prop->minlevel = atol(str[4].c_str());
			prop->maxcount = atol(str[5].c_str());
			prop->price = atol(str[6].c_str());
			prop->maxuse = atol(str[7].c_str());
			prop->issplit = atol(str[8].c_str());
			prop->isdestroy = atol(str[9].c_str());
			prop->istransaction = atol(str[10].c_str());
			prop->isshow = atol(str[11].c_str());
			prop->isbind = atol(str[12].c_str());
			prop->issell = atol(str[13].c_str());
			prop->tab = atol(str[14].c_str());

			//更新脚本指针 避免在游戏中再来查找 搞高性能
			prop->equip.reset();
			prop->consume.reset();
			if (prop->type == app::EPT_EQUIP)
			{
				auto value = findScript_Equip(prop->id);
				if (value != NULL) memcpy(&prop->equip, value, sizeof(SCRIPT_PROP_EQUIP));
			}
			else if (prop->type == app::EPT_CONSUME)
			{
				auto value = findScript_Consume(prop->id);
				if (value != NULL) memcpy(&prop->consume, value, sizeof(SCRIPT_PROP_CONSUME));
			}

			__ScriptProps.insert(std::make_pair(prop->id, prop));
		}
	}

	void loadScript_Map()
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath, "%scsv\\%s", func::FileExePath, "map.csv");
		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			LOG_MESSAGE("loadScript_map.csv iserror... \n");
			return;
		}
		bool isfirstblv = false;

		//1、读取map表
		while (fgets(lineblv, 1000, fpblv))
		{
			if (!isfirstblv)
			{
				isfirstblv = true;
				continue;
			}
			std::string countstr = lineblv;
			size_t n = countstr.find_last_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(n + 1, countstr.size() - n);
			}

			n = countstr.find_first_not_of(" \r\n\t");
			if (n != std::string::npos)
			{
				countstr.erase(0, n);
			}

			SCRIPT_MAP_BASE* map = new SCRIPT_MAP_BASE();
			std::vector<std::string> str = share::split(countstr, ",", true);
			if (str.size() != 12) continue;

			map->mapid = atol(str[0].c_str());
			map->maptype = atol(str[1].c_str());
			map->row = atol(str[2].c_str());
			map->col = atol(str[3].c_str());
			map->copy_player = atol(str[4].c_str());
			map->copy_level = atol(str[5].c_str());
			map->copy_difficulty = atol(str[6].c_str());
			map->copy_win = atol(str[7].c_str());
			map->copy_limittime = atol(str[8].c_str()) * 1000;
			map->isreborn = atol(str[11].c_str());

			if (map->mapid < 0 || map->mapid >= MAX_MAP_ID) continue;
			__ScriptMaps[map->mapid] = map;
		}
	}

	void initScript()
	{
		for (int i = 0; i < MAX_SKILLSCRIPT_COUNT; i++)
			__ScriptSkills[i] = nullptr;
		for (int i = 0; i < MAX_BUFFSCRIPT_COUNT; i++)
			__ScriptBuffs[i] = nullptr;
		for (int i = 0; i < MAX_LEVELSCRIPT_COUNT; i++)
			__ScriptLevels[i] = nullptr;
		for (int i = 0; i < MAX_MONSTERSCRIPT_COUNT; i++) 
			__ScriptMonsters[i] = nullptr;
		for (int i = 0; i < MAX_MAP_ID; i++) 
			__ScriptMaps[i] = nullptr;
		
		loadScript_Skill();
		loadScript_Buff();
		loadScript_Monster();
		loadScript_Level();

		loadScript_Equip();//加载装备
		loadScript_Consume();//加载消耗类道具
		loadScript_Prop();//加载道具表

		loadScript_Map();
	}
}

//判断道具有效性 使用道具的时候来判断
bool app::S_ROLE_PROP::isPropValid()
{
	if (this->IsT() == false) return false;
	if (temp.script_prop != NULL) return true;
	auto script = script::findScript_Prop(this->base.id);
	if (script == NULL) return false;
	temp.script_prop = script;
	return true;
}