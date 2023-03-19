#include "AppRobot.h"
#include "WorldData.h"
#include "RobotManager.h"
#include "AppGlobal.h"
#include "WorldBC.h"
#include "UserManager.h"
namespace app
{
	AppRobot* __AppRobot = nullptr;

	AppRobot::AppRobot()
	{

	}

	AppRobot::~AppRobot()
	{

	}

	void onData(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_ROBOT_DATA data;
		ts->read(c->ID, &data, sizeof(S_ROBOT_DATA));

// 		auto user = __UserManager->findUser(data.userindex);
// 		if (user == nullptr)
// 		{
// 			sendErrInfo(ts, c->ID, CMD_8000, 3001, &data, sizeof(S_ROBOT_DATA));
// 			return;
// 		}
		
		S_ROBOT* robot = __RobotManager->findRobot(data.robotindex);
		if (robot == NULL)
		{
			LOG_MESSAGE("AppRobot 8000 err ...%d line:%d\n", data.robotindex, __LINE__);
			sendErrInfo(ts, c->ID, CMD_8000, 3002, &data, sizeof(S_ROBOT_DATA));
			return;
		}
// 		if (user->role.base.status.mapid != robot->data.status.mapid)
// 		{
// 			// 地图不一样
// 			LOG_MESSAGE("AppRobot 8000 err ...%d line:%d\n", data.robotindex, __LINE__);
// 			sendErrInfo(ts, c->ID, CMD_8000, 3003, &data, sizeof(S_ROBOT_DATA));
// 			return;
// 		}
		LOG_MESSAGE("send AppRobot 8000  ... %d\n", data.robotindex);
		ts->begin(c->ID, CMD_8000);
		ts->sss(c->ID, (u16)0);
		ts->sss(c->ID, &data, sizeof(S_ROBOT_DATA));
		ts->sss(c->ID, robot->data.status.id);
		ts->sss(c->ID, robot->data.status.dir);
		ts->sss(c->ID, robot->data.status.hp);
		ts->sss(c->ID, &robot->bc.grid_pos, 8);
		ts->end(c->ID);

		for (u32 i = 0; i < MAX_BUFF_COUNT; i++)
		{
			auto buff = &robot->tmp.temp_buffRun.data[i];
			if (buff->buff_id <= 0) continue;
			if (buff->runningtime <= 200) continue;
			TS_Broadcast::do_SendBuff(CMD_8800, c->ID, data.user_connectid, data.memid, robot->node.index, buff);
		}
	}

	bool AppRobot::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppMove err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_8000:
			onData(ts, c);
			break;
		}
		return true;
	}

}