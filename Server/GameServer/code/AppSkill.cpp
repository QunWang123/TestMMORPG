#include "AppSkill.h"
#include "CommandData.h"
#include "UserManager.h"
#include "AppGlobal.h"
#include "WorldScript.h"
#include "WorldBC.h"
#include "WorldMap.h"
#include "WorldTools.h"
namespace app
{
	AppSkill* __AppSkill = nullptr;

	void sendErrInfo(s32 connectID, const u16 cmd, const u16 childcmd, S_SKILL_ROLE* data)
	{
		__TcpServer->begin(connectID, cmd);
		__TcpServer->sss(connectID, childcmd);
		__TcpServer->sss(connectID, data, sizeof(S_SKILL_ROLE));
		__TcpServer->end(connectID);
	}

	void sendErrInfo(s32 connectID, const u16 cmd, const u16 childcmd, S_REBORN_ROLE* data)
	{
		__TcpServer->begin(connectID, cmd);
		__TcpServer->sss(connectID, childcmd);
		__TcpServer->sss(connectID, data, sizeof(S_REBORN_ROLE));
		__TcpServer->end(connectID);
	}

	AppSkill::AppSkill()
	{

	}

	AppSkill::~AppSkill()
	{

	}

	void onSkill(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_SKILL_ROLE data;
		ts->read(c->ID, &data, sizeof(S_SKILL_ROLE));

		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user == NULL)
		{
			sendErrInfo(c->ID, CMD_3000, 3001, &data);
			return;
		}
		// 验证技能标号是否合法
		auto skill = user->skill_IsSafe(data.skillindex);
		if (skill == nullptr)
		{
			sendErrInfo(c->ID, CMD_3000, 3002, &data);
			return;
		}
		// 获取技能
		auto skillscript = script::findScript_Skill(skill->id, skill->level);
		if (skillscript == nullptr)
		{
			sendErrInfo(c->ID, CMD_3000, 3003, &data);
			return;
		}
		// 公共CD时间
		bool isTime = user->skill_IsPublicTime();
		if (!isTime)
		{
			sendErrInfo(c->ID, CMD_3000, 3004, &data);
			return;
		}
		// 验证CD时间
		isTime = user->skill_IsColdTime(data.skillindex, skillscript->coldtime);
		if (!isTime)
		{
			sendErrInfo(c->ID, CMD_3000, 3005, &data);
			return;
		}
		// 玩家是否活着
		if (!user->isLive())
		{
			sendErrInfo(c->ID, CMD_3000, 3006, &data);
			return;
		}
		// 验证技能锁定目标，锁定的是空且这个技能要求不能空锁		
		if (data.lock_type == N_FREE && skillscript->islock != 0)
		{
			sendErrInfo(c->ID, CMD_3000, 3007, &data);
			return;
		}
		// 验证职业
		if (user->role.base.innate.job != skillscript->needjob)
		{
			sendErrInfo(c->ID, CMD_3000, 3008, &data);
			return;
		}
		// 验证能否使用技能（受眩晕等DEBUFF影响）
		if (!user->tmp.temp_BuffRun.isUseSkill())
		{
			sendErrInfo(c->ID, CMD_3000, 3009, &data);
			return;
		}

		// 验证mp点
		if (user->role.base.life.mp < skillscript->needmp)
		{
			sendErrInfo(c->ID, CMD_3000, 3010, &data);
			return;
		}

		// 直接生成buff的技能
		if (skillscript->type == E_SKILL_USE_TYPE_BUFF)
		{
			TS_Tools::buff_skill_create(user, skillscript, N_ROLE);
			// user->buff_skill_create(skillscript);
			return;
		}

		// 1.开始播放技能
		s32 err = user->skill_start(&data, skillscript);
		if (err == 0) return;

		sendErrInfo(c->ID, CMD_3000, err, &data);
	}

	void onReborn(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_REBORN_ROLE data;
		ts->read(c->ID, &data, sizeof(S_REBORN_ROLE));

		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user == NULL)
		{
			sendErrInfo(c->ID, CMD_4100, 3001, &data);
			return;
		}

		if (user->role.base.life.hp > 0)
		{
			sendErrInfo(c->ID, CMD_4100, 3002, &data);
			return;
		}

		user->role.base.life.hp = 100;
		user->role.base.status.state = E_SPRITE_STATE::E_SPRITE_STATE_LIVING;

		S_VECTOR3 targetpos;
		targetpos = user->role.base.status.pos;

		// 附近城镇复活
		if (data.kind > 0)
		{
			
// 			S_WORLD_MAP map = __World->map[user->role.base.status.mapid];
// 			TS_Tools::gridToPos(map.Pos.getReborn(), &targetpos, &map.leftpos);

// 			targetpos.x = 80;
// 			targetpos.y = -350;
// 			targetpos.z = 278;

			user->initReBornPos(user->role.base.status.mapid);
			targetpos = user->role.base.status.pos;

			// 判断是否跨越九宫格
			bool isupdate = false;
			user->moveWorld(&targetpos, isupdate);
			if (isupdate)
			{
				TS_Broadcast::bc_RoleMoveLeaveBig(&user->bc.edgeold, user);
			}
		}
		TS_Broadcast::bc_RoleReborn(user, &targetpos);
	}

	bool AppSkill::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppMove err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_3000:
			onSkill(ts,c);
			break;
		case CMD_4100:
			onReborn(ts, c);
			break;
		}
		return true;
	}
}