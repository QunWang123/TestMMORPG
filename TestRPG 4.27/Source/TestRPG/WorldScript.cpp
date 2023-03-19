#include "WorldScript.h"
// #include "../../share/ShareFunction.h"
#include "Misc/Paths.h"

bool isInitPath = false;
// 字符串分割函数
std::vector<std::string> split(std::string str, std::string patten, bool isadd = false)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	if (isadd) str += patten;

	int size = str.size();
	for (int i = 0; i < size; i++)
	{
		pos = str.find(patten, i);
		if (pos < size)
		{
			std::string s = str.substr(i, pos - i);	// pos为起始位置（默认0），n为结束位置，默认（npos）
			result.push_back(s);
			i = pos + patten.size() - 1;
		}
	}
	return result;
}
// 初始化路径
void InitPath()
{
	FString basedir = FPaths::ProjectDir();
	basedir = FPaths::Combine(*basedir, TEXT("Binaries/Win64/"));

	TCHAR* pSendData = basedir.GetCharArray().GetData();
	int nDataLen = FCString::Strlen(pSendData);
	char* dst = (char*)TCHAR_TO_UTF8(pSendData);

	memset(FileExePath, 0, MAX_EXE_LEN);
	memcpy(FileExePath, dst, MAX_EXE_LEN);

	isInitPath = true;
}

namespace script
{
	SCRIPT_SKILL_BASE*		__ScriptSkills[MAX_SKILLSCRIPT_COUNT];
	SCRIPT_BUFF_BASE*		__ScriptBuffs[MAX_BUFFSCRIPT_COUNT];
	SCRIPT_MONSTER_BASE*	__ScriptMonsters[MAX_MONSTERSCRIPT_COUNT];
	SCRIPT_MAP_BASE*		__ScriptMaps[MAX_MAP_COUNT];
#pragma optimize("", off)
	void loadScript_Skill()
	{
		char fpath[MAX_EXE_LEN];
		memset(fpath, 0, MAX_EXE_LEN);
		sprintf_s(fpath, "%scsv\\%s", FileExePath, "skill.csv");

		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			// LOG_MESSAGE("skill.csv is error... \n");
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

			std::vector<std::string> str = split(countstr, ",", true);
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

				uint8* cc = (uint8*)FMemory::Malloc(str[22].length());
				skill->name = UTF8_TO_TCHAR(cc);
				FMemory::Free(cc);

				uint32 index = skill->id * 4 + skill->level * 100;
				if (index >= MAX_SKILLSCRIPT_COUNT)
				{
					delete skill;
					continue;
				}
//   				FString ss = FString::Printf(TEXT("skill...%d-%d"), skill->id, skill->type);
//  				ss = ss + skill->name;
//   				if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 1115, FColor::Emerald, ss);
				__ScriptSkills[index] = skill;
			}
		}
	}

	void loadScript_Buff()
	{
		char fpath[MAX_EXE_LEN];
		memset(fpath, 0, MAX_EXE_LEN);
		sprintf_s(fpath, "%scsv\\%s", FileExePath, "buff.csv");

		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			// LOG_MESSAGE("skill.csv is error... \n");
			return;
		}

		bool isfirstblv = false;

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

			std::vector<std::string> str = split(countstr, ",", true);
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

				// uint8* cc = (uint8*)FMemory::Malloc(str[8].length());
				buff->name = UTF8_TO_TCHAR(str[8].c_str());
				// FMemory::Free(cc);

				if (buff->id >= MAX_BUFFSCRIPT_COUNT)
				{
					delete buff;
					continue;
				}
//   				FString ss = FString::Printf(TEXT("buff...%d-%d"), buff->id, buff->state);
// 				ss = ss + buff->name;
//  				if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 1115, FColor::Emerald, ss);
				__ScriptBuffs[buff->id] = buff;
			}
		}
	}

	void loadScript_Monster()
	{
		char fpath[MAX_EXE_LEN];
		memset(fpath, 0, MAX_EXE_LEN);
		sprintf_s(fpath, "%scsv\\%s", FileExePath, "monster.csv");

		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			// LOG_MESSAGE("skill.csv is error... \n");
			return;
		}

		bool isfirstblv = false;

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

			std::vector<std::string> str = split(countstr, ",", true);
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

				std::vector<std::string> tmpdropstr = split(str[16].c_str(), ";", true);
				for (std::string t : tmpdropstr)
				{
					std::vector<std::string> arr = split(t.c_str(), "|", true);
					if (arr.size() != 3) continue;
					SCRIPT_DROP_ID* did = new SCRIPT_DROP_ID();
					did->id = atol(arr[0].c_str());
					did->probability = atol(arr[1].c_str());
					did->count = atol(arr[2].c_str());
					monster->drop_ids.push_back(did);
				}

				std::vector<std::string> tmpskillstr = split(str[17].c_str(), ";", true);
				if (tmpskillstr.size() > MAX_SKILL_COUNT)
				{
					delete monster;
					continue;
				}
				for (std::string t : tmpskillstr)
				{
					monster->skill_ids.push_back(atoi(t.c_str()));
				}

				monster->model_id = atoi(str[18].c_str());

// 				std::string ss = str[19];
// 				int size = ss.size();
// 				uint8* cc = (uint8*)FMemory::Malloc(size);
// 				FMemory::Memset(cc, 0, size);
// 				memcpy(cc, ss.c_str(), size);
// 				monster->nick = UTF8_TO_TCHAR(cc);
// 				FMemory::Free(cc);

				monster->nick = UTF8_TO_TCHAR(str[19].c_str());

				
				if (monster->id >= MAX_MONSTERSCRIPT_COUNT || monster->skill_ids.size() < 1)
				{
					delete monster;
					continue;
				}
				__ScriptMonsters[monster->id] = monster;
			}
		}
	}

	void loadScript_Map()
	{
		char fpath[MAX_EXE_LEN];
		memset(fpath, 0, MAX_EXE_LEN);
		sprintf_s(fpath, "%scsv\\%s", FileExePath, "map.csv");

		FILE* fpblv = fopen(fpath, "r");
		char lineblv[1000];
		if (fpblv == nullptr)
		{
			// LOG_MESSAGE("skill.csv is error... \n");
			return;
		}

		bool isfirstblv = false;

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
			std::vector<std::string> str = split(countstr, ",", true);

			if (str.size() == 12)
			{
				SCRIPT_MAP_BASE* map = new SCRIPT_MAP_BASE();
				map->mapid = atol(str[0].c_str());
				map->maptype = atol(str[1].c_str());
				map->row = atol(str[2].c_str());
				map->col = atol(str[3].c_str());
				map->copy_player = atol(str[4].c_str());
				map->copy_level = atol(str[5].c_str());
				map->copy_difficulty = atol(str[6].c_str());
				map->copy_win = atol(str[7].c_str());
				map->copy_limittime = atol(str[8].c_str()) * 1000;
				map->isreborn = atoi(str[11].c_str());

// 				uint8* cc = (uint8*)FMemory::Malloc(str[9].length());
// 				memcpy(cc, str[9].c_str(), str[9].length());
// 				map->mapname = UTF8_TO_TCHAR(cc);
// 				FMemory::Free(cc);

				std::string ss = str[9];
				map->mapname = UTF8_TO_TCHAR(ss.c_str());

				if (map->mapid >= MAX_MAP_COUNT)
				{
					delete map;
					continue;
				}
				//   				FString ss = FString::Printf(TEXT("buff...%d-%d"), buff->id, buff->state);
				// 				ss = ss + buff->name;
				//  				if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 1115, FColor::Emerald, ss);
				__ScriptMaps[map->mapid] = map;
			}
		}
	}
#pragma optimize("", on)
	void initScript()
	{
		if (isInitPath) return;
		InitPath();

		for (int i = 0; i < MAX_SKILLSCRIPT_COUNT; i++)
			__ScriptSkills[i] = nullptr;
		loadScript_Skill();
		for (int i = 0; i < MAX_BUFFSCRIPT_COUNT; i++)
			__ScriptBuffs[i] = nullptr;
		loadScript_Buff();
		for (int i = 0; i < MAX_MONSTERSCRIPT_COUNT; i++)
			__ScriptMonsters[i] = nullptr;
		loadScript_Monster();
		for (int i = 0; i < MAX_MAP_COUNT; i++)
			__ScriptMaps[i] = nullptr;
		loadScript_Map();
	}

	SCRIPT_SKILL_BASE* fingScript_Skill(int32 id, int32 level)
	{
		if (id <= 0 || level <= 0) return nullptr;
		int32 index = id * 4 + level * 100;
		if (index >= MAX_SKILLSCRIPT_COUNT) return nullptr;
		return __ScriptSkills[index];
	}

	extern SCRIPT_BUFF_BASE* fingScript_Buff(int32 id)
	{
		if (id <= 0 || id >= MAX_BUFFSCRIPT_COUNT) return nullptr;
		return __ScriptBuffs[id];
	}

	extern SCRIPT_MONSTER_BASE* findScript_Monster(int32 id)
	{
		if (id <= 0 || id >= MAX_MONSTERSCRIPT_COUNT) return nullptr;
		return __ScriptMonsters[id];
	}

	extern SCRIPT_MAP_BASE* findScript_Map(int32 id)
	{
		if (id <= 0 || id >= MAX_MAP_COUNT) return nullptr;
		return __ScriptMaps[id];
	}
}


