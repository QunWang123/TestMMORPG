#include "UdpServer.h"

namespace net
{
	void UdpServer::runThread()
	{
		m_workthread.reset(new std::thread(UdpServer::run, this));
		m_workthread->detach();
	}







	void UdpServer::run(UdpServer* udp)
	{
		printf("UDP run workthread....\n");

		//更新
		while (true)
		{
			//udp->sendData("127.0.0.1", 9001, "test linux...");

			int err = udp->recvData();
			if (err >= 0) continue;

			printf("为了防止死循环 暂停1毫秒...\n");
			UDPSleep(2);
		}
	}





	


}