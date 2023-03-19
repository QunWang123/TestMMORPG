#include "AppEnterWorld.h"
#include "CommandData.h"
#include "CommandData2.h"
#include "UserManager.h"
#include "WorldData.h"
#include "WorldBC.h"

namespace app
{
	AppEnterWorld* __AppEnterWorld = nullptr;

	AppEnterWorld::AppEnterWorld()
	{

	}

	AppEnterWorld::~AppEnterWorld()
	{

	}

	void AppEnterWorld::onInit()
	{

	}


	//2、进入世界
	void onEntryWorld(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_CMD_USER_ENTRY data;
		ts->read(c->ID, &data, sizeof(S_CMD_USER_ENTRY));

		if (__UserManager == nullptr) return;
		S_USER_TEAM* user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			LOG_MESSAGE("onEntryWorld  user = null...userindex:%d/%lld \n", data.userindex, data.memid);
			// io::pushLog(io::EFT_RUN, "onEntryWorld  user = null...userindex:%d/%lld \n", data.userindex, data.memid);
			return;
		}

		//玩家账号ID
		user->mem.state = E_MEMBER_STATE::M_LOGINEND;
		user->mem.id = data.memid;
		user->status.mapid = data.mapid;
		user->role.level = data.level;
		user->tmp.line = data.line;
		user->tmp.userindex = data.userindex;
		user->tmp.user_connectid = data.user_connectid;
		user->tmp.server_connectid = c->ID;
		user->tmp.server_clientid = c->clientID;
		user->tmp.temp_HeartTime = time(NULL);

		memcpy(user->mem.name, data.name, USER_MAX_MEMBER);
		memcpy(user->role.nick, data.nick, USER_MAX_NICK);

		__UserManager->__OnlineCount++;

		// 		memset(temp_str, 0, 1024);
		// 		sprintf_s(temp_str, "onEntryWorld name:%s nick:%s id:%d/%lld  mapid:%d/%d connectid:%d\n",
		// 			user->mem.name, user->role.nick, data.userindex, user->mem.id, user->tmp.line, user->status.mapid, data.user_connectid);
				// LOG_MESSAGE("%s", temp_str);
				// io::pushLog(io::EFT_RUN, "%s", temp_str);
	}

	//3、切换地图
	void onChangeMap(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_CHANGEMAP_BASE data;
		ts->read(c->ID, &data, sizeof(S_CHANGEMAP_BASE));

		if (__UserManager == nullptr) return;
		S_USER_TEAM* user = __UserManager->findUser(data.userindex, data.memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onChangeMap  user = null...userindex:%d/%lld \n", data.userindex, data.memid);
			// io::pushLog(io::EFT_RUN, "onChangeMap  user = null...userindex:%d/%lld \n", data.userindex, data.memid);
			return;
		}

		user->status.mapid = data.mapid;
		user->tmp.line = data.line;

		//7100 通知gameserver 更新自己的队伍信息
		nf_GameServer_TeamSelf(user);

		LOG_MESSAGE("[GameServer]  onChangeMap...index:%d/%lld mapid:%d:%d\n", data.userindex, data.memid, data.mapid, data.line);
	}


	bool AppEnterWorld::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppEnterWorld err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_900:
			onEntryWorld(ts, c);
			break;
		case CMD_901:
		case CMD_902:
			onChangeMap(ts, c);
			break;
		}
		return true;
	}
}