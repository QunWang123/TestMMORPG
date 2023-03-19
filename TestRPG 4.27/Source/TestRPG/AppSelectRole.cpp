#include "AppSelectRole.h"
#include "AppManager.h"
#include "UserData.h"
#include "BlueprintFunction.h"
#include "MyGameInstance.h"

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

	void AppSelectRole::onUpdate()
	{

	}

	void AppSelectRole::send_CreateRole(uint8 injob, uint8 insex, FString innick)
	{
		__TcpClient->begin(CMD_300);
		__TcpClient->sss(__myUserData.memid);
		__TcpClient->sss(injob);
		__TcpClient->sss(insex);
		UBlueprintFunction::send_FString_len(innick, USER_MAX_NICK);
		__TcpClient->end();
	}

	void AppSelectRole::send_DeleteRole(uint8 roleindex)
	{
		if (roleindex >= USER_MAX_ROLE) return;

		auto role = &rolebase[roleindex];
		__TcpClient->begin(CMD_400);
		__TcpClient->sss(__myUserData.memid);
		__TcpClient->sss(roleindex);
		__TcpClient->sss(role->id);
		__TcpClient->end();
	}
	// 200 ѡ���ɫ��ʼ��Ϸ
	void AppSelectRole::send_StartGame(uint8 roleindex)
	{
		__myUserData.select_roleindex = roleindex;
		app::__TcpClient->begin(CMD_200);
		app::__TcpClient->sss(roleindex);
		app::__TcpClient->sss(__myUserData.memid);
		app::__TcpClient->end();
	}

	// ���ܻ��յ�һЩ������Ϣ
	void onCMD_200(net::TCPClient* tc)
	{
		uint16 childcmd;
		tc->read(childcmd);

		if (childcmd != 0)
		{
			if (__AppGameInstance != nullptr) __AppGameInstance->onSelectRoleCommand(CMD_200, childcmd, 99);
			return;
		}
	}

	void onCMD_300(net::TCPClient* tc)
	{
		uint16 childcmd;
		tc->read(childcmd);

		if (childcmd != 0)
		{
			if (__AppGameInstance != nullptr) __AppGameInstance->onSelectRoleCommand(CMD_300, childcmd, 99);
			return;
		}

		uint8 roleindex;
		tc->read(roleindex);
		if (roleindex >= USER_MAX_ROLE) return;

		auto role = &rolebase[roleindex];
		tc->read(role->id);
		tc->read(role->job);
		tc->read(role->sex);
		role->nick = UBlueprintFunction::read_FString_len(USER_MAX_NICK);
		if (__AppGameInstance != nullptr) __AppGameInstance->onSelectRoleCommand(CMD_300, childcmd, roleindex);
	}

	void onCMD_400(net::TCPClient* tc)
	{
		uint16 childcmd;
		tc->read(childcmd);
		
		if (childcmd != 0)
		{
			if (__AppGameInstance != nullptr) __AppGameInstance->onSelectRoleCommand(CMD_400, childcmd, 99);
			return;
		}

		uint8 roleindex;
		tc->read(roleindex);
		if (roleindex >= USER_MAX_ROLE) return;

		FMemory::Memset(&rolebase[roleindex], 0, 22);
		rolebase[roleindex].nick = "";
		if (__AppGameInstance != nullptr) __AppGameInstance->onSelectRoleCommand(CMD_400, childcmd, roleindex);
	}

	void onCMD_500(net::TCPClient* tc)
	{
		uint16 childcmd;
		tc->read(childcmd);

		if (childcmd != 0)
		{
			if (__AppGameInstance != nullptr) __AppGameInstance->onSelectRoleCommand(CMD_500, childcmd, 99);
			return;
		}

		tc->getData()->state = func::C_LOGIN;
		__myUserData.reset();
		
		uint16	level = 0;
		int16	face = 0;
		tc->read(level);
		tc->read(__myUserData.base.exp.currexp);
		__myUserData.base.exp.level = level;
		// ����
		tc->read(&__myUserData.base.econ, sizeof(FROLE_BASE_ECON));
		// ״̬
		tc->read(__myUserData.base.status.state);
		tc->read(face);
		__myUserData.base.status.face = face;
		tc->read(__myUserData.base.status.mapid);
		tc->read(&__myUserData.base.status.pos, 12);
		// ������Ϣ
		tc->read(&__myUserData.base.life, sizeof(FROLE_BASE_LIFE));
		// ��ȡ���userindex
		tc->read(__myUserData.userindex);
		// ��ȡ��Ҽ���
		for (int i = 0; i < MAX_SKILL_COUNT; i++)
		{
			tc->read(&__myUserData.stand.myskill.skill[i], sizeof(FROLE_STAND_SKILL_BASE));
		}

		InitMapInfo(__myUserData.base.status.mapid);

		if (__myUserData.select_roleindex < USER_MAX_ROLE)
		{
			FMemory::Memcpy(&__myUserData.rolebase, &rolebase[__myUserData.select_roleindex], 22);
			__myUserData.rolebase.nick = rolebase[__myUserData.select_roleindex].nick;
		}

		if (__AppGameInstance != nullptr) __AppGameInstance->onSelectRoleCommand(CMD_500, childcmd, 99);
	}

	// �յ����ط�������������Ϣ
	bool AppSelectRole::onClientCommand(net::TCPClient* tc, const uint16 cmd)
	{
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
		case CMD_500:
			onCMD_500(tc);
			break;
		}
		return true;
	}

}
