#include "WorldScript.h"
#include "ShareFunction.h"
#include <map>
using namespace share;
using namespace std;
namespace script
{
	SCRIPT_MAP_BASE* __ScriptMaps[MAX_MAP_ID];


	void loadScript_Map();
	//初始化全部脚本
	void initScript()
	{
		for (int i = 0; i < MAX_MAP_ID; i++) __ScriptMaps[i] = nullptr;

		loadScript_Map();
	}

	SCRIPT_MAP_BASE* findScript_Map(s32 id)
	{
		if (id < 0 || id >= MAX_MAP_ID) return nullptr;
		return __ScriptMaps[id];
	}

	bool isCopyMap(s32 mapid)
	{
		auto s = findScript_Map(mapid);
		if (s == nullptr) return true;
		if (s->maptype == 0) return false;
		return true;
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
			string countstr = lineblv;
			size_t n = countstr.find_last_not_of(" \r\n\t");
			if (n != string::npos)
			{
				countstr.erase(n + 1, countstr.size() - n);
			}

			n = countstr.find_first_not_of(" \r\n\t");
			if (n != string::npos)
			{
				countstr.erase(0, n);
			}

			SCRIPT_MAP_BASE* map = new SCRIPT_MAP_BASE();
			std::vector<std::string> str = split(countstr, ",",true);
			if (str.size() != 12) continue;

			map->mapid = atol(str[0].c_str());
			map->maptype = atol(str[1].c_str());
			map->row = atol(str[2].c_str());
			map->col = atol(str[3].c_str());
			map->copy_player = atol(str[4].c_str());
			map->copy_level = atol(str[5].c_str());
			map->copy_difficulty = atol(str[6].c_str());
			map->copy_win = atol(str[7].c_str());
			map->copy_limittime = atol(str[8].c_str()) * 100000;
			map->isreborn = atol(str[11].c_str());;

			if (map->mapid < 0 || map->mapid >= MAX_MAP_ID) continue;
			__ScriptMaps[map->mapid] = map;
		}
	}
}

