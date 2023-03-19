#include "UdpClient.h"
#include "KcpInterface.h"
namespace net
{
	int kcpOutput_client(const char* buf, int size, ikcpcb* kcp, void* user)
	{
		S_UDP_BASE* c = (S_UDP_BASE*)user;
		//发送信息
		int sendBytes = sendto(c->socketfd, buf, size, 0, (struct sockaddr*)&c->addr, sizeof(struct sockaddr_in));
		if (sendBytes >= 0)
		{
			//会重复发送，因此牺牲带宽
			printf("kcpOutput_client-send: 字节 =%d / %d bytes   \n", sendBytes, size);//24字节的KCP头部
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

	//设置KCP
	void UdpClient::createKcp(s32 id)
	{
		releaseKcp();
		//创建KCP
		ikcpcb* kcp = ikcp_create(id, &data);
		kcp->output = kcpOutput_client;
		kcp->rx_minrto = 10;
		//ikcp_nodelay(kcp, 0, 10, 0, 0);//1, 10, 2, 1
		ikcp_nodelay(kcp, 1, 10, 2, 1);//1, 10, 2, 1
		ikcp_wndsize(kcp, 128, 128);

		data.kcp = kcp;
	}



	//kcp 解析数据
	int UdpClient::recvData_kcp(char* buf, s32 recvBytes)
	{
		//2 验证是不是KCP通信
		if (recvBytes < 24) return recvBytes;

		int id = ikcp_getid(buf);
		if (id != data.ID)
		{
			LOG_MESSAGE("kcp err...ID不一致 %d/%d \n", id, data.ID);
			return recvBytes;
		}

		//获取真实数据
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
		////验证解析出来的真实数据 
		//if (readBytes < 6)
		//{
		//	LOG_MSG("kcp err...readBytes<0 %d/%d \n", id, readBytes);
		//	return recvBytes;
		//}

		//onRecv_SaveData(buf_temp, &data, recvBytes);
		return recvBytes;
	}

}