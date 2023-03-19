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
		//5秒去发送一次数据，是主动发送的
		s32 value = time(NULL) - temp_Time;
		if (value < 5) return;
		temp_Time = time(NULL);

		//1 上传网关服务器内存 在线数据
		if (__TcpCenter->getData()->state >= func::C_CONNECT_SECURE)
		{
			s32 concount = 0;
			s32 securtiycount = 0;
			__TcpServer->getSecurityCount(concount, securtiycount);

			u32 curmemory = 0;
			u32 maxmemory = 0;
			share::updateMemory(curmemory, maxmemory);
			
			__TcpCenter->begin(CMD_40);
			__TcpCenter->sss(func::__ServerInfo->ID); //服务器ID
			__TcpCenter->sss((u8)0);//线
			__TcpCenter->sss(securtiycount);//安全连接数量
			__TcpCenter->sss(curmemory);//内存
			__TcpCenter->end();
		}

		//2 申请游戏服务器数据,一次给中心服务器只发一个游戏服务器的内容
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
			//说明游戏服务器失去连接
			__TcpCenter->begin(CMD_40);
			__TcpCenter->sss(tcpgame->getData()->serverID); //服务器ID
			__TcpCenter->sss((u8)0);
			__TcpCenter->sss(0);//在线玩家
			__TcpCenter->sss(0);//内存
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
		__TcpCenter->sss(id); //服务器ID
		__TcpCenter->sss(line); //所在线
		__TcpCenter->sss(online);//在线玩家
		__TcpCenter->sss(memory);//内存
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