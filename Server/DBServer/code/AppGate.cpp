#include "AppGate.h"
#include <algorithm>
#include <time.h>
#include "AppGlobal.h"
#include "ShareFunction.h"
#include "UserManager.h"

using namespace func;

namespace app
{
	AppGate* __AppGate = nullptr;

	AppGate::AppGate()
	{
	}
	AppGate::~AppGate()
	{
	}
	void AppGate::onUpdate()
	{
	}

	//login gate
	void onGetDBList(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);

		u32 count = __UserManager->__MemberStates.size();

		ts->begin(c->ID, CMD_40);
		ts->sss(c->ID, func::__ServerInfo->ID);
		ts->sss(c->ID, count);
		ts->sss(c->ID, curmemory);
		ts->end(c->ID);

	}

	bool AppGate::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppLogin err...line:%d \n", __LINE__);
			return false;
		}
		if (c->clientType != func::S_TYPE_CENTER) return false;
		if (cmd != CMD_40) return false;

		onGetDBList(ts, c);
		return true;
	}
}