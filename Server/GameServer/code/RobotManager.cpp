#include "RobotManager.h"
#include "WorldMap.h"
#include "WorldTools.h"
#include "WorldScript.h"


namespace app
{
	RobotManager* __RobotManager = NULL;


	RobotManager::RobotManager()
	{
		init();
	}

	RobotManager::~RobotManager()
	{

	}

	void RobotManager::init()
	{
		robots_ = new HashArray<S_ROBOT>(MAX_SPRITE_COUNT);
		createRobot();
	}

	void RobotManager::update()
	{
		if (checkCount <= 0) return;
		// if (checkPos >= MAX_SPRITE_COUNT) checkPos = 0;
		if (checkPos >= MAX_SPRITE_COUNT) checkPos = func::__ServerInfo->MaxUser;

		s32 num = checkPos + checkCount;
		if (num > MAX_SPRITE_COUNT) num = MAX_SPRITE_COUNT;

		for (u32 i = checkPos; i < num; i++)
		{
			S_ROBOT* robot = robots_->Value(i);
			robot->event_AI(-1);

		}
		checkPos += checkCount;
	}

	void RobotManager::reset()
	{
		count = 0;
		// 使用MaxUser是为了避免和玩家下标重复
		emptyPos = func::__ServerInfo->MaxUser;
		checkPos = func::__ServerInfo->MaxUser;
		// 每一次检查的数量
		checkCount = (MAX_SPRITE_COUNT - func::__ServerInfo->MaxUser) / 10;
	}

	void RobotManager::createRobot()
	{
		reset();
		// u32 i = 0或者i = func::__serverinfo_maxuser是一样的效果
		for (u32 i = 0; i < MAX_SPRITE_COUNT; i++)
		{
			S_ROBOT* robot = robots_->Value(i);
			robot->init(N_FREE, i);
		}

		s32 collide_id = 0;
		u32 sciptindex = -1;
		script::SCRIPT_MONSTER_BASE* script = nullptr;

		for (u32 mapid = 0; mapid < MAX_MAP_ID; mapid++)
		{
			S_WORLD_MAP* map = &__World->map[mapid];
			if (!map->isUsed) continue;

			script::SCRIPT_MAP_BASE* script_map = script::findScript_Map(mapid);
			if (script_map == nullptr) continue;
			if (script_map->maptype != EMT_PUBLIC_MAP) continue;


			u32  max_row = map->Data.max_row;
			u32  max_col = map->Data.max_col;


			for (u32 row = 0; row < max_row; row++)
			{
				for (u32 col = 0; col < max_col; col++)
				{
					collide_id = map->Data.collides[row][col];

					// 地图编辑器编辑，属于怪物
					if (collide_id < 50 || collide_id > 200) continue;

					collide_id = 10000 + (collide_id - 100) + 1;

					script = script::findScript_Monster(collide_id);
					if (script == nullptr)
					{
						LOG_MESSAGE("怪物脚本=null:[%d-%d] %d \n", row, col, collide_id);
						continue;
					}
					if (emptyPos >= MAX_SPRITE_COUNT)
					{
						LOG_MESSAGE("怪物数量超过上限:[%d-%d] %d \n", row, col, collide_id);
						return;
					}


					S_ROBOT* robot = Robot(emptyPos);
					//1 设置节点信息
					robot->node.type = N_MONSTER;
					robot->node.layer = -1;
					//robot->node.index = emptyPos;
					//2 设置状态数据
					robot->data.status.mapid = mapid;
					robot->data.status.id = script->id;
					robot->data.status.state = E_SPRITE_STATE_FREE;
					robot->data.status.hp = script->maxhp;
					robot->data.status.mp = script->maxmp;
					robot->data.status.born_pos.row = row;
					robot->data.status.born_pos.col = col;
					robot->data.status.dir = TS_Tools::Random(8);

					//3 设置脚本数据
					robot->tmp.script = script;
					// 4.将机器人添加进世界，失败了则将他初始化
					bool issuccess = robot->createInWorld(mapid, row, col);
					if (issuccess)
					{
						robot->data.status.isreborn = script_map->isreborn;
						robot->data.ai.randTime(robot->node.index);
						emptyPos++;
						count++;

						// LOG_MESSAGE("createRobot mapid:%d :[%d-%d] %d index:%d\n", mapid, row, col, collide_id,robot->node.index);

					}
					else
						robot->init(N_FREE, robot->node.index);
				}
			}
		}
	}

	s32 RobotManager::createRobot_Copy(const u32 mapid, const s32 layer)
	{
		s32 collide_id = 0;
		u32 sciptindex = -1;
		auto script_map = script::findScript_Map(mapid);
		if (script_map == nullptr) return -1;
		if (script_map->maptype == EMT_PUBLIC_MAP) return -2;

		script::SCRIPT_MONSTER_BASE* script = nullptr;

		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return -3;
		if (!map->isUsed)   return -4;

		u32  max_row = map->Data.max_row;
		u32  max_col = map->Data.max_col;

		for (u32 row = 0; row < max_row; row++)
			for (u32 col = 0; col < max_col; col++)
			{
				collide_id = map->Data.collides[row][col];

				if (collide_id < 50 || collide_id > 200) continue;

				collide_id = 10000 + (collide_id - 100) + 1;
				script = script::findScript_Monster(collide_id);
				if (script == nullptr)
				{
					LOG_MESSAGE("怪物脚本=null:[%d-%d] %d \n", row, col, collide_id);
					continue;
				}
				S_ROBOT* robot = findEmpty();
				if (robot == nullptr)
				{
					LOG_MESSAGE("copy 怪物数量超过上限:[%d-%d] %d \n", row, col, collide_id);
					return -5;
				}

				//1 设置节点信息
				robot->node.type = N_MONSTER;
				robot->node.layer = layer;
				//2 设置状态数据
				robot->data.status.mapid = mapid;
				robot->data.status.id = script->id;
				robot->data.status.state = E_SPRITE_STATE_FREE;
				robot->data.status.hp = script->maxhp;
				robot->data.status.mp = script->maxmp;
				robot->data.status.born_pos.row = row;
				robot->data.status.born_pos.col = col;
				robot->data.status.dir = TS_Tools::Random(8);

				//3 设置脚本数据
				robot->tmp.script = script;

				bool issuccess = robot->createInWorld(mapid, row, col);
				if (issuccess)
				{
					robot->data.status.isreborn = script_map->isreborn;
					robot->data.ai.randTime(robot->node.index);
					count++;

					//LOG_MSG("createRobot copy mapid:%d :[%d-%d] %d index:%d\n", mapid, row, col, collide_id, robot->node.index);
				}
				else
					robot->init(N_FREE, robot->node.index);
			}
		return 0;
	}

	void RobotManager::clearRobot_Copy(const u32 mapid, const s32 layer)
	{
		LOG_MESSAGE("clearRobot_Copy...mapid:%d layer:%d \n", mapid, layer);

		//0-emptyPos 是公共地图的怪物
		for (u32 i = emptyPos; i < MAX_SPRITE_COUNT; i++)
		{
			S_ROBOT* robot = Robot(i);
			if (robot == nullptr) continue;
			if (robot->data.status.mapid == mapid && robot->node.layer == layer)
			{
				if (robot->bc.isinWorld)
				{
					robot->leaveWorld();
					count--;
					if (count <= 0) count = 0;
				}
				robot->init(N_FREE, i);
			}
		}
	}

	app::S_ROBOT* RobotManager::findRobot(const u32 index)
	{
		S_ROBOT* robot = Robot(index);
		if (robot == nullptr || !robot->isT()) return nullptr;
		return robot;
	}

	app::S_ROBOT* RobotManager::findRobot(const u32 index, const s32 layer)
	{
		S_ROBOT* robot = Robot(index);
		if (robot == nullptr || !robot->isT() || robot->node.layer != layer) return nullptr;
		return robot;
	}

	app::S_ROBOT* RobotManager::findRobot(S_RECT_BASE* edge, const u32 index, const s32 layer, const u32 mapid)
	{
		S_ROBOT* robot = findRobot(index, layer);
		if (robot == nullptr || 
			robot->data.status.mapid != mapid || 
			!robot->isLive() || 
			!edge->inEdge(&robot->bc.grid_big)) return nullptr;
		return robot;
	}

	app::S_ROBOT* RobotManager::findEmpty()
	{
		if (emptyPos >= MAX_SPRITE_COUNT) return nullptr;
		for (u32 i = emptyPos; i < MAX_SPRITE_COUNT; i++)
		{
			S_ROBOT* robot = Robot(i);
			if (robot == nullptr || robot->isT()) continue;

			return robot;
		}
		return nullptr;
	}

}
