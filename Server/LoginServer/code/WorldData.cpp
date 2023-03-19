#include "WorldData.h"
#include "AppGlobal.h"

namespace app
{
	void sendErrInfo(const s32 connectid, const u32 cmd, const u16 childcmd)
	{
		__TcpServer->begin(connectid, cmd);
		__TcpServer->sss(connectid, childcmd);
		__TcpServer->end(connectid);
	}
}