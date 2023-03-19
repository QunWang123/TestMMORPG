#include "IDefine_Udp.h"

void net::S_UDP_BASE::init(int32 length)
{
	recvBuf = new uint8[length];
	threadID = -1;
	ID = -1;
	ip = 0;
	port = 0;
	kcp = NULL;
	UdpSocket = NULL;

	reset();
}
void net::S_UDP_BASE::reset()
{
	state = 0;
	is_RecvCompleted = true;
	memset(recvBuf, 0, func::__UDPClientInfo->RecvMax);
	recv_Head = 0;
	recv_Tail = 0;
	recv_TempHead = 0;
	recv_TempTail = 0;
	time_Heart = 0;
	time_AutoConnect = 0;
	time_HeartConnect = 0;
}


