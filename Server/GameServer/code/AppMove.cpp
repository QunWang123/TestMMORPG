#include "AppMove.h"
#include "CommandData.h"
#include "UserManager.h"
#include "AppGlobal.h"
#include "WorldTools.h"
#include "WorldBC.h"
namespace app
{
	AppMove* __AppMove = nullptr;

	void sendErrInfo(s32 connectID, const u16 cmd, const u16 childcmd, S_MOVE_ROLE* data)
	{
		__TcpServer->begin(connectID, cmd);
		__TcpServer->sss(connectID, childcmd);
		__TcpServer->sss(connectID, data, sizeof(S_MOVE_ROLE));
		__TcpServer->end(connectID);
	}

	AppMove::AppMove()
	{

	}

	AppMove::~AppMove()
	{

	}

	void AppMove::onInit()
	{

	}

	void onMove(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_MOVE_ROLE move;

		ts->read(c->ID, &move, sizeof(S_MOVE_ROLE));

		auto user = __UserManager->findUser(move.userindex, move.memid);
		if (user == NULL)
		{
			sendErrInfo(c->ID, CMD_1000, 3001, &move);
			return;
		}

		user->role.base.status.face = move.face;

		// ���Z������0���Ǿ͹�������
		// ����ͼ��ʱ������Z�������
		if (user->role.base.status.pos.z != 0)
		{
			int kind = user->isMove(&move.curpos);
			int distance = TS_Tools::getDistance(&move.curpos, &user->role.base.status.pos);
			if ((distance > 200 || kind != 0))
			{
				if (!user->tmp.bChangeMap)
				{
					move.targetpos = user->role.base.status.pos;
					sendErrInfo(c->ID, CMD_1000, 3000, &move);
					return;
				}
				user->tmp.bChangeMap = false;
			}
		}

		
		// ���������ƶ�
		bool isupdata = false;
		user->moveWorld(&move.curpos, isupdata);

		if (isupdata)
		{
			// �㲥�뿪����
			TS_Broadcast::bc_RoleMoveLeaveBig(&user->bc.edgeold, user);
		}

		// �㲥�ƶ�
		TS_Broadcast::bc_RoleMove(user, &move);
	}

	bool AppMove::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppMove err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_1000:
			onMove(ts, c);
			break;
		}

		return true;
	}
}