#include "WorldTools.h"
#include "RobotData.h"
#include <time.h>
#include "WorldBC.h"
#include "WorldScript.h"

namespace app
{
	void TS_Tools::buff_skill_create(void* sprite, void* sk, const u8 type)
	{
		S_USER_BASE* user = NULL;
		S_ROBOT* robot = NULL;
		S_TEMP_BUFF_RUN* buff = NULL;

		switch (type)
		{
		case N_ROLE:
			user = (S_USER_BASE*)sprite;
			buff = &user->tmp.temp_BuffRun;
			break;
		case N_MONSTER:
		case N_PET:
		case N_NPC:
			robot = (S_ROBOT*)sprite;
			buff = &robot->tmp.temp_buffRun;
			break;
		default:
			return;
		}

		if (buff == NULL) return;

		script::SCRIPT_SKILL_BASE* skill_script = (script::SCRIPT_SKILL_BASE*)sk;
		// 技能不会生成buff
		if (skill_script->buff_id <= 0)
		{
			LOG_MESSAGE("buff_skill_create err...buff_id = %d line:%d\n", skill_script->buff_id, __LINE__);
			return;
		}
		// S_TEMP_BUFF_RUN_BASE* buff_run = buff->findBuff(skill_script->buff_id);
		S_TEMP_BUFF_RUN_BASE* buff_run = buff->findFree();

		script::SCRIPT_BUFF_BASE* buff_script = script::findScript_Buff(skill_script->buff_id);
		if (buff_run == nullptr || buff_script == nullptr)
		{
			LOG_MESSAGE("buff_skill_create err...buff_id = %d line:%d\n", skill_script->buff_id, __LINE__);
			return;
		}
		//添加BUFF状态
		buff->addState(buff_script->state);
// 		s32 value = buff_script->state & E_BUFF_NOMOVE; //是不是可以移动
// 		if (value == E_BUFF_NOMOVE) buff->addState(E_BUFF_NOMOVE);
// 		value = buff_script->state & E_BUFF_NOMOVE; //是不是可以使用技能
// 		if (value == E_BUFF_NOSKILL) buff->addState(E_BUFF_NOSKILL);

		//覆盖buff
		buff_run->buff_id = buff_script->id;
		buff_run->runningtime = buff_script->runningtime;
		buff_run->temptime = clock();


		if (type == N_ROLE)
		{
			if (user) TS_Broadcast::bc_RoleBuff(user, buff_run);
			//LOG_MSG("buff add...%lld:%d-%d \n", this->mem.id, buff_run->buff_id, this->tmp.temp_BuffRun.state);
		}
		else
		{
			//LOG_MSG("robot buff add...%lld:%d-%d \n", robot->node.index, buff_run->buff_id, robot->tmp.temp_BuffRun.state);
			if (robot) TS_Broadcast::bc_RobotBuff(robot, buff_run);
		}
		//LOG_MSG("buff add...%lld:%d-%d \n", this->mem.id, buff_run->buff_id, this->tmp.temp_BuffRun.state);
	}

	void TS_Tools::buff_runing(void* sprite, const u8 type, const s32 value)
	{
		S_USER_BASE* user = NULL;
		S_ROBOT* robot = NULL;
		S_TEMP_BUFF_RUN* buff = NULL;
		s32 id = 0;
		switch (type)
		{
		case N_ROLE:
			user = (S_USER_BASE*)sprite;
			buff = &user->tmp.temp_BuffRun;
			id = user->mem.id;
			break;
		case N_MONSTER:
		case N_PET:
		case N_NPC:
			robot = (S_ROBOT*)sprite;
			buff = &robot->tmp.temp_buffRun;
			id = robot->node.index;
			break;
		default:
			return;
		}
		if (buff == NULL) return;

		bool isCheck = false;
		for (u32 i = 0; i < MAX_BUFF_COUNT; i++)
		{
			S_TEMP_BUFF_RUN_BASE* buff_run = &buff->data[i];
			if (buff_run->buff_id <= 0) continue;

			//	LOG_MSG("check buff..%d %d %d   value:%d\n", id, buff_run->buff_id, buff_run->runningtime,value);

			buff_run->runningtime -= value;
			if (buff_run->runningtime > 0) continue;

			int value = clock() - buff_run->temptime;

			LOG_MESSAGE("buff delete...%d-%d %d time:%d\n", id, buff_run->buff_id, buff_run->runningtime, value);
			buff_run->reset();
			//检查BUFF状态
			isCheck = true;
		}

		if (isCheck == false) return;
		// 如果是叠加的不能移动的话
		// 有一个不能移动失效了，另外一个也会跟着被清0
		// 所以要判断剩余的buff里面还是不是含有这个状态
		s32 state = 0;
		for (u32 i = 0; i < MAX_BUFF_COUNT; i++)
		{
			S_TEMP_BUFF_RUN_BASE* run = &buff->data[i];
			if (run->buff_id <= 0) continue;
			script::SCRIPT_BUFF_BASE* buff = script::findScript_Buff(run->buff_id);
			if (buff == NULL) continue;
			state = state | buff->state;
		}
		buff->state = state;
	}

	void TS_Tools::buff_skill_random(void* sprite, void* sk, const u8 type)
	{
		script::SCRIPT_SKILL_BASE* skill_script = (script::SCRIPT_SKILL_BASE*)sk;

		if (skill_script->buff_id <= 0) return;
		s32 buff_percent = TS_Tools::RandomRange(0, 100);
		bool isbuff = (buff_percent < skill_script->buff_percent) ? true : false;

		if (isbuff) buff_skill_create(sprite, sk, type);
	}
}

