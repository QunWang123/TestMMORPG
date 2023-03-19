#include "UdpServer.h"
#include "KcpInterface.h"

using namespace func;


namespace net
{

	//***********************************************************
	//1 ��������........

	int kcpOutput_server(const char* buf, int size, ikcpcb* kcp, void* user)
	{
		S_UDP_BASE* c = (S_UDP_BASE*)user;
		//������Ϣ
		int sendBytes = sendto(c->socketfd, buf, size, 0, (struct sockaddr*)&c->addr, sizeof(struct sockaddr_in));
		if (sendBytes >= 0)
		{
			//���ظ����ͣ������������
			printf("kcpOutput_ id:%d  sendBytes=%d / %d bytes   \n",c->ID, sendBytes, size);//24�ֽڵ�KCPͷ��
			return sendBytes;
		}
		else
		{
			printf("error: %d bytes send, error\n", sendBytes);
			return -1;
		}
	}

	s32 nextUpdateFrame = 0;
	void UdpServer::updateKcp(S_UDP_BASE* c)
	{
		if (c->kcp == nullptr) return;
		ikcp_update(c->kcp, iclock());

		//u32 curtime = iclock();
		//if (nextUpdateFrame == 0 || curtime >= nextUpdateFrame)
		//{
		//	ikcp_update(c->kcp, curtime);
		//	nextUpdateFrame = ikcp_check(c->kcp, curtime);
		//}
	}


	//***********************************************************
    //2 ����kcp����........
	void UdpServer::createKcp(S_UDP_BASE* c)
	{
		releaseKcp(c);

		//����KCP
		ikcpcb* kcp = ikcp_create(c->ID, c);
		kcp->output = kcpOutput_server;
		kcp->rx_minrto = 10;
		//ikcp_nodelay(kcp, 0, 10, 0, 0);//1, 10, 2, 1
		ikcp_nodelay(kcp, 1, 10, 2, 1);//1, 10, 2, 1
		ikcp_wndsize(kcp, 128, 128);

		c->kcp = kcp;
	}
	//�ͷ�kcp
	void UdpServer::releaseKcp(S_UDP_BASE* c)
	{
		if (c->kcp == nullptr) return;
		ikcp_release(c->kcp);
		c->kcp = nullptr;
	}

	//***********************************************************
    //��������........
	int UdpServer::recvData_kcp(char* buf, s32 recvBytes, s32 ip, u16 port)
	{
		//2 ��֤�ǲ���KCPͨ�� 24+6 
		if (recvBytes < 24) return recvBytes;

		int id = ikcp_getid(buf);
		auto c = findClient(id, ip, port);
		if (c == NULL)
		{
			//LOG_MSG("kcp err...c=null %d \n", id);
			return recvBytes;
		}

		//��ȡ��ʵ����
		int ret = ikcp_input(c->kcp, buf, recvBytes);
		if (recvBytes <= 24 || ret < 0) return recvBytes;

		while (true)
		{
			char buf_temp[MAX_UDP_BUF];
			memset(buf_temp, 0, MAX_UDP_BUF);

			//KCPͨ��ikcp_recv�����ݽ��ճ������������Ƭ���ͣ����ڴ��Զ����飬���ݽ��뷢��ǰ����һ�¡�
			ret = ikcp_recv(c->kcp, buf_temp, MAX_UDP_BUF);
			if (ret >= 6)  onRecv_SaveData(buf_temp, c, ret);
			else if (ret < 0) break;
		}
		return recvBytes;
	}
}