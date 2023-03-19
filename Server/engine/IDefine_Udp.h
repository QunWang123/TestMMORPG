#ifndef __IDEFINE_UDP_H
#define __IDEFINE_UDP_H

#include "IDefine.h"
#include <vector>

#ifdef ___WIN32_
//#include <winsock2.h>
//#pragma comment(lib,"ws2_32") 

//#include <WS2tcpip.h>

typedef SOCKET UDPSOCKET;
#define UDPSleep(value) Sleep(value)
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)

#else
#include <cstdio>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>

typedef int UDPSOCKET;
#define UDPSleep(value) usleep(value*1000)

#endif

#include <string>
#include <thread> 





//#define LOG_MSG printf 

//#define MAX_USER_SOCKETFD   1000000 
//#define MAX_EXE_LEN         200      
//#define MAX_FILENAME_LEN    250     
//#define SIO_KEEPALIVE_VALS  IOC_IN | IOC_VENDOR | 4 
//#define MAX_MD5_LEN  35 
//#define MAX_IP_LEN   20 
//#define MAX_COMMAND_LEN     65535
//#define MAX_IP_ONE_COUNT    20
#define MAX_UDP_BUF   512
#define MAX_VALUE   1000000 

#define CMD_65001   65001
#define CMD_65002   65002
#define CMD_65003   65003

//typedef signed char          s8;
//typedef signed short         s16;
//typedef signed int           s32;
//typedef unsigned char        u8;
//typedef unsigned short       u16;
//typedef unsigned int         u32;
//typedef float		f32;
//typedef double		f64;
//
//#ifdef ____WIN32_
//typedef signed long long     s64;
//typedef unsigned long long   u64;
//#else
//typedef signed long int     s64;
//typedef unsigned long int   u64;
//
//#endif

//#define this_constructor(struct_type) struct_type(){ memset(this, 0, sizeof(struct_type)); }


namespace func
{
#pragma pack(push,packing)
#pragma pack(1)

	struct S_DATA_HEAD
	{
		s32   ID;
		u16   cmd;
		inline void setSecure(u32 code)
		{
			ID = ID ^ code;
			cmd = cmd ^ code;
		}
		inline void getSecure(u32 code)
		{
			ID = ID ^ code;
			cmd = cmd ^ code;
		}
	};
	struct S_DATA_BASE
	{
		char head[2];
		u16  length;
		char buf[MAX_UDP_BUF];
		inline void reset()
		{
			memset(this, 0, sizeof(S_DATA_BASE));
		}
	};

	struct S_DATA_TEST22 :public S_DATA_HEAD
	{
		this_constructor(S_DATA_TEST22)

		u32   a;
		u16   b;
		char  s[20];
		u8    state;
	};

#pragma pack(pop, packing)







	//extern void loadWindowsDll();
	extern int getError();
	extern void setNonblockingSocket(bool isNon, UDPSOCKET socketfd);

	//extern char FileExePath[MAX_EXE_LEN];
	extern ConfigXML* __UDPServerInfo;
	extern ConfigXML* __UDPClientInfo;
	extern std::vector<ServerListXML*> __UDPServerListInfo;
	//extern void(*MD5str)(char* output, unsigned char* input, int len);
	//extern bool InitData();

	//extern void setConsoleColor(u16 index);
	extern void formatTime(time_t time1, char* szTime);

}
#endif 