#include "AppGate.h"
#include <algorithm>
#include <time.h>
#include "AppGlobal.h"
#include "ShareFunction.h"
namespace app
{
	AppGate* __AppGate = nullptr;
	u32  temp_Time = 0;
	u32  temp_TcpGameIndex = 0;

	AppGate::AppGate()
	{
	}
	AppGate::~AppGate()
	{
	}
	
	void AppGate::onUpdate()
	{
		//5��ȥ����һ�����ݣ����������͵�
		s32 value = time(NULL) - temp_Time;
		if (value < 5) return;
		temp_Time = time(NULL);

		//1 �ϴ����ط������ڴ� ��������
		if (__TcpCenter->getData()->state >= func::C_CONNECT_SECURE)
		{
			s32 concount = 0;
			s32 securtiycount = 0;
			__TcpServer->getSecurityCount(concount, securtiycount);

			u32 curmemory = 0;
			u32 maxmemory = 0;
			share::updateMemory(curmemory, maxmemory);
			
			__TcpCenter->begin(CMD_40);
			__TcpCenter->sss(func::__ServerInfo->ID); //������ID
			__TcpCenter->sss((u8)0);//��
			__TcpCenter->sss(securtiycount);//��ȫ��������
			__TcpCenter->sss(curmemory);//�ڴ�
			__TcpCenter->end();
		}

		//2 ������Ϸ����������,һ�θ����ķ�����ֻ��һ����Ϸ������������
		if (__TcpGame.size() == 0) return;
		if (temp_TcpGameIndex >= __TcpGame.size()) temp_TcpGameIndex = 0;
		auto tcpgame = __TcpGame[temp_TcpGameIndex];
		if (tcpgame == nullptr) return;
			
		if(tcpgame->getData()->state >= func::C_CONNECT_SECURE)
		{
			tcpgame->begin(CMD_40);
			tcpgame->end();
		}
		else
		{
			//˵����Ϸ������ʧȥ����
			__TcpCenter->begin(CMD_40);
			__TcpCenter->sss(tcpgame->getData()->serverID); //������ID
			__TcpCenter->sss((u8)0);
			__TcpCenter->sss(0);//�������
			__TcpCenter->sss(0);//�ڴ�
			__TcpCenter->end();
		}
		temp_TcpGameIndex++;
	}
	
	void onCMD_40(net::ITCPClient* tc)
	{
		s32 id = 0;
		s32 online = 0;
		s32 memory = 0;
		u8 line = 0;

		tc->read(id);
		tc->read(line);
		tc->read(online);
		tc->read(memory);
		
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE) return;

		__TcpCenter->begin(CMD_40);
		__TcpCenter->sss(id); //������ID
		__TcpCenter->sss(line); //������
		__TcpCenter->sss(online);//�������
		__TcpCenter->sss(memory);//�ڴ�
		__TcpCenter->end();
	}
	
	bool AppGate::onClientCommand(net::ITCPClient* tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_GAME) return false;
		if (cmd != CMD_40) return false;

		onCMD_40(tc);
		return true;
	}
}