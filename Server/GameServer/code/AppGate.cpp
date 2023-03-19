#include "AppGate.h"


#include "AppGlobal.h"
#include "UserManager.h"
#include "ShareFunction.h"
using namespace func;

namespace app
{
	AppGate* __AppGate = nullptr;

	u32  temp_Time = 0;
	AppGate::AppGate()
	{
	}
	AppGate::~AppGate()
	{
	}
	void AppGate::onUpdate()
	{
	}
	void onGetGateList(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);

		ts->begin(c->ID, CMD_40);
		ts->sss(c->ID, func::__ServerInfo->ID); //������ID
		ts->sss(c->ID, share::__ServerLine);//������������
		ts->sss(c->ID, __UserManager->__OnlineCount);//��������
		ts->sss(c->ID, curmemory);//�ڴ�
		ts->end(c->ID);

	}

	bool AppGate::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppLogin err...line:%d \n", __LINE__);
			return false;
		}
		if (c->clientType != func::S_TYPE_GATE) return false;
		if (cmd != CMD_40) return false;

		onGetGateList(ts, c);
		return false;
	}
}