
#include "INetBase_Udp.h"


namespace func
{
	//全局变量
	//char FileExePath[MAX_EXE_LEN];//EXE执行路径
	ConfigXML* __UDPServerInfo = nullptr;//服务器配置信息
	ConfigXML* __UDPClientInfo = nullptr;//客户端配置信息
	std::vector<ServerListXML*> __UDPServerListInfo;//服务器列表信息
	//void(*MD5str)(char* output, unsigned char* input, int len) = NULL;


	//void loadWindowsDll()
	//{
	//#ifdef ____WIN32__
	//	//1、初始化Windows Sockets DLL
	//	WSADATA  wsData;
	//	int error = WSAStartup(MAKEWORD(2, 2), &wsData);
	//	if (error != 0)
	//	{
	//		perror("WSAStartup error！");
	//		exit(1);
	//	}
	//#endif // ____WIN32__
	//}
	//获取错误原因
	int getError()
	{
	#ifdef ___WIN32_
		return WSAGetLastError();
	#else
		return errno;
	#endif
	}
	//设置非阻塞SOCKET isNon true
	void setNonblockingSocket(bool isNon, UDPSOCKET socketfd)
	{
		if (isNon == false) return;

	#ifdef ___WIN32_
		unsigned long ul = 1;
		ioctlsocket(socketfd, FIONBIO, (unsigned long*)&ul);
	#else
		int flags = fcntl(socketfd, F_GETFL);
		if (flags < 0) return;
		flags |= O_NONBLOCK;
		if (fcntl(socketfd, F_SETFL, flags) < 0) return;
	#endif
	}
//	bool InitData()
//	{
//		memset(FileExePath, 0, MAX_EXE_LEN);
//#ifdef ____WIN32_
//		//1、初始化路径 win32_API
//		GetModuleFileNameA(NULL, (LPSTR)FileExePath, MAX_EXE_LEN);
//
//		std::string str(FileExePath);
//		size_t pos = str.find_last_of("\\");
//		str = str.substr(0, pos + 1);
//
//		memcpy(FileExePath, str.c_str(), MAX_EXE_LEN);
//
//		printf("FileExePath:%s \n", FileExePath);
//#else
//		int ret = readlink("/proc/self/exe", FileExePath, MAX_EXE_LEN);
//		std::string str(FileExePath);
//		size_t pos = str.find_last_of("/");
//		str = str.substr(0, pos + 1);
//		memcpy(FileExePath, str.c_str(), MAX_EXE_LEN);
//
//		printf("FileExePath:%s \n", FileExePath);
//#endif
//
//		return true;
//	}
	void formatTime(time_t time1, char* szTime)
	{
		struct tm tm1;
#ifdef ___WIN32_
		tm1 = *localtime(&time1);
#else  
		localtime_r(&time1, &tm1);
#endif  
		sprintf(szTime, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
			tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday,
			tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	}
//	void setConsoleColor(u16 index)
//	{
//#ifdef ____WIN32_
//		//0 = 黑色   1 = 蓝色   2 = 绿色  3 = 浅绿色 4 = 红色 5 = 紫色 6 = 黄色
//		//7 = 白色   8 = 灰色   9 = 淡蓝色 A = 淡绿色 B = 淡浅绿色  C = 淡红色
//		//D = 淡紫色 E = 淡黄色 F = 亮白色
//		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), index);
//#endif
//	}
}

namespace net
{
	void S_UDP_BASE::init(u32 length)
	{
		memid = 0;
		recvBuf = new char[length];
		kcp = NULL;
		reset();
	}

	void S_UDP_BASE::reset()
	{
		threadID = -1;
		ID = -1;
		ip = 0;
		port = 0;
		memset(&addr, 0, sizeof(sockaddr_in));

		initdata();
	}
	void S_UDP_BASE::initdata()
	{
		state = 0;
		memid = 0;
		is_RecvCompleted = true;
		memset(recvBuf, 0, func::__UDPServerInfo->RecvMax);
		recv_Head = 0;
		recv_Tail = 0;
		recv_TempHead = 0;
		recv_TempTail = 0;
		time_Heart = (int)time(NULL);
		time_HeartConnect = time(NULL);
		time_AutoConnect = (int)time(NULL);
	}
}

