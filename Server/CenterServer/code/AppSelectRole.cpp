#include "AppSelectRole.h"
#include "UserManager.h"
#include "WorldData.h"
#include "AppGlobal.h"

namespace app
{
	AppSelectRole* __AppSelectRole = nullptr;

	AppSelectRole::AppSelectRole()
	{

	}

	AppSelectRole::~AppSelectRole()
	{

	}

	void AppSelectRole::onInit()
	{

	}

	void onSelectRole(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_SELECT_ROLE* role = __UserManager->popRequestPool();
		ts->read(c->ID, role, sizeof(S_SELECT_ROLE));
		role->server_clientid = c->clientID;
		role->server_connectid = c->ID;

		// ��֤�������Ƿ����ӵ���db������
		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			// û���Ͼ͸���Ӧ�Ŀͻ��˷�һ������ȥ
			sendErrInfo(c->ID, CMD_200, 2001, role, sizeof(S_SELECT_ROLE));
			__UserManager->pushRequestPool(role);
			return;
		}
		// �������
		__UserManager->insertRequest(role);
	}

	void onCreateRole(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_CREATE_ROLE role;
		ts->read(c->ID, &role, sizeof(S_CREATE_ROLE));

		role.server_clientid = c->clientID;
		role.server_connectid = c->ID;

		// ��֤�������Ƿ����ӵ���db������
		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			// û���Ͼ͸���Ӧ�Ŀͻ��˷�һ������ȥ
			sendErrInfo(c->ID, CMD_300, 2001, &role, sizeof(S_CREATE_ROLE));
			return;
		}

		__TcpDB->begin(CMD_300);
		__TcpDB->sss(&role, sizeof(S_CREATE_ROLE));
		__TcpDB->end();
	}

	void onDeleteRole(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_DELETE_ROLE role;
		ts->read(c->ID, &role, sizeof(S_DELETE_ROLE));

		role.server_clientid = c->clientID;
		role.server_connectid = c->ID;

		// ��֤�������Ƿ����ӵ������ķ�����
		if (__TcpDB->getData()->state < func::C_CONNECT_SECURE)
		{
			// û���Ͼ͸���Ӧ�Ŀͻ��˷�һ������ȥ
			sendErrInfo(c->ID, CMD_400, 2001, &role, sizeof(S_DELETE_ROLE));
			return;
		}

		__TcpDB->begin(CMD_400);
		__TcpDB->sss(&role, sizeof(S_DELETE_ROLE));
		__TcpDB->end();
	}

	bool AppSelectRole::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppSelectRole err ... line:%d", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_GATE) return false;

		switch (cmd)
		{
		case CMD_200:
			onSelectRole(ts, c);
			break;
		case CMD_300:
			onCreateRole(ts, c);
			break;
		case CMD_400:
			onDeleteRole(ts, c);
			break;
		}
		return true;
	}

	/*******************************************************************************************/

	void onCMD_200(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_SELECT_ROLE role;
		tc->read(childcmd);
		tc->read(&role, sizeof(S_SELECT_ROLE));

		//LOG_MSG("200 �յ�DB��ʼ��Ϸ....memid:%lld/%d err:%d\n", role.memid, role.user_connectid, childcmd);

		//������������ ����
		__UserManager->setIsRequesting(false);

		net::S_CLIENT_BASE* c = __TcpServer->client(role.server_connectid, role.server_clientid);
		if (c == NULL)
		{
			LOG_MESSAGE("AppSelectRole err...c == null [%d-%d] %d\n", 
				role.server_connectid, role.server_clientid, __LINE__);
			return;
		}
		//���ظ���ҽ�ɫ��������
		sendErrInfo(c->ID, CMD_200, childcmd, &role, sizeof(S_SELECT_ROLE));
	}

	void onCMD_300(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_CREATE_ROLE role;
		tc->read(childcmd);
		tc->read(&role, sizeof(S_CREATE_ROLE));

		auto c = __TcpServer->client(role.server_connectid, role.server_clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("AppSelectRole err c == NULL [%d-%lld] line:%d\n", 
				role.server_connectid, role.server_clientid, __LINE__);
			return;
		}

		// ���ܳɹ�ʧ�ܶ�ֱ�ӷ������
		sendErrInfo(c->ID, CMD_300, childcmd, &role, sizeof(S_CREATE_ROLE));

	}

	void onCMD_400(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_DELETE_ROLE role;
		tc->read(childcmd);
		tc->read(&role, sizeof(S_DELETE_ROLE));

		auto c = __TcpServer->client(role.server_connectid, role.server_clientid);
		if (c == nullptr)
		{
			LOG_MESSAGE("AppSelectRole err c == NULL [%d-%lld] line:%d\n",
				role.server_connectid, role.server_clientid, __LINE__);
			return;
		}

		// ���ܳɹ�ʧ�ܶ�ֱ�ӷ������
		sendErrInfo(c->ID, CMD_400, childcmd, &role, sizeof(S_DELETE_ROLE));
	}

	bool AppSelectRole::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_DB) return false;

		switch (cmd)
		{
		case CMD_200:
			onCMD_200(tc);
			break;
		case CMD_300:
			onCMD_300(tc);
			break;
		case CMD_400:
			onCMD_400(tc);
			break;
		}
		return true;
	}
}



