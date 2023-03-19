#include "UdpClient.h"

namespace net
{
	void UdpClient::runThread()
	{
		m_workthread.reset(new std::thread(UdpClient::run, this));
		m_workthread->detach();
	}







	void UdpClient::run(UdpClient* udp)
	{
		printf("run workthread....%d \n",udp->getData()->threadID);
		udp->sendData(udp->data.strip,udp->data.port, "",0);

		//����
		while (true)
		{
			//udp->sendData("127.0.0.1", 9001, "test linux...");

			int err = udp->recvData();
			if (err >= 0) continue;

			//printf("Ϊ�˷�ֹ��ѭ�� ��ͣ1����...\n");
			UDPSleep(2);
		}
	}
}