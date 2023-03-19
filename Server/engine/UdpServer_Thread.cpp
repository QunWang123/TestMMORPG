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

		//����
		while (true)
		{
			//udp->sendData("127.0.0.1", 9001, "test linux...");

			int err = udp->recvData();
			if (err >= 0) continue;

			printf("Ϊ�˷�ֹ��ѭ�� ��ͣ1����...\n");
			UDPSleep(2);
		}
	}





	


}