#include "UdpClient.h"
#include "KcpInterface.h"
namespace net
{
	int kcpOutput_client(const char* buf, int size, ikcpcb* kcp, void* user)
	{
		S_UDP_BASE* c = (S_UDP_BASE*)user;
		//������Ϣ
		int sendBytes = sendto(c->socketfd, buf, size, 0, (struct sockaddr*)&c->addr, sizeof(struct sockaddr_in));
		if (sendBytes >= 0)
		{
			//���ظ����ͣ������������
			printf("kcpOutput_client-send: �ֽ� =%d / %d bytes   \n", sendBytes, size);//24�ֽڵ�KCPͷ��
			return sendBytes;
		}
		else
		{
			printf("error: %d bytes send, error\n", sendBytes);
			return -1;
		}
	}
	void UdpClient::updateKcp()
	{
		if (data.kcp == nullptr) return;
		ikcp_update(data.kcp, iclock());
	}

	void UdpClient::releaseKcp()
	{
		if (data.kcp == nullptr) return;
		ikcp_release(data.kcp);
		data.kcp = nullptr;
	}

	//����KCP
	void UdpClient::createKcp(s32 id)
	{
		releaseKcp();
		//����KCP
		ikcpcb* kcp = ikcp_create(id, &data);
		kcp->output = kcpOutput_client;
		kcp->rx_minrto = 10;
		//ikcp_nodelay(kcp, 0, 10, 0, 0);//1, 10, 2, 1
		ikcp_nodelay(kcp, 1, 10, 2, 1);//1, 10, 2, 1
		ikcp_wndsize(kcp, 128, 128);

		data.kcp = kcp;
	}



	//kcp ��������
	int UdpClient::recvData_kcp(char* buf, s32 recvBytes)
	{
		//2 ��֤�ǲ���KCPͨ��
		if (recvBytes < 24) return recvBytes;

		int id = ikcp_getid(buf);
		if (id != data.ID)
		{
			LOG_MESSAGE("kcp err...ID��һ�� %d/%d \n", id, data.ID);
			return recvBytes;
		}

		//��ȡ��ʵ����
		//int readBytes = 0;

		int ret = ikcp_input(data.kcp, buf, recvBytes);
		if (recvBytes <= 24 || ret < 0) return recvBytes;
		while (true)
		{
			char buf_temp[MAX_UDP_BUF];
			memset(buf_temp, 0, MAX_UDP_BUF);
			ret = ikcp_recv(data.kcp, buf_temp, MAX_UDP_BUF);
			if (ret >= 6) onRecv_SaveData(buf_temp, &data, ret);
			else if (ret < 0) break;
		}
		////��֤������������ʵ���� 
		//if (readBytes < 6)
		//{
		//	LOG_MSG("kcp err...readBytes<0 %d/%d \n", id, readBytes);
		//	return recvBytes;
		//}

		//onRecv_SaveData(buf_temp, &data, recvBytes);
		return recvBytes;
	}

}