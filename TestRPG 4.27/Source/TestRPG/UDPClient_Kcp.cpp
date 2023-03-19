#include "UdpClient.h"
#include "MyGameInstance.h"

int kcpOutput_client(const char* buf, int size, ikcpcb* kcp, void* user)
{
	S_UDP_BASE* c = (S_UDP_BASE*)user;
	if (c == NULL || c->UdpSocket == NULL) return -1;

	static TSharedPtr<FInternetAddr>	addr_kcp;
	if (!addr_kcp.IsValid())
	{
		addr_kcp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	}
	bool isValid = false;
	addr_kcp->SetIp(*(c->strip), isValid);
	addr_kcp->SetPort(c->port);

	//发送信息
	int32 sendBytes = 0;
	bool err = c->UdpSocket->SendTo((uint8*)buf, size, sendBytes, *addr_kcp);

	return sendBytes;
}
void UDPClient::updateKcp()
{
	if (data.kcp == nullptr) return;

	int value = __AppGameInstance->GetTimeSeconds();
	ikcp_update(data.kcp, value);
}

void UDPClient::releaseKcp()
{
	if (data.kcp == nullptr) return;
	ikcp_release(data.kcp);
	data.kcp = nullptr;
}

//设置KCP
void UDPClient::createKcp(int32 id)
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
int UDPClient::recvData_kcp(char* buf, int32 recvBytes)
{
	//2 验证是不是KCP通信
	if (recvBytes < 24) return recvBytes;

	int id = ikcp_getid(buf);
	if (id != data.ID)
	{
		//LOG_MSG("kcp err...ID不一致 %d/%d \n", id, data.ID);
		return recvBytes;
	}

	//获取真实数据
	//int readBytes = 0;
	//char buf_temp[512];
	//memset(buf_temp, 0, 512);
	int ret = ikcp_input(data.kcp, buf, recvBytes);
	if (recvBytes <= 24) return recvBytes;
	while (true)
	{
		char buf_temp[512];
		memset(buf_temp, 0, 512);
		ret = ikcp_recv(data.kcp, buf_temp, 512);
		if (ret > 0)
		{
			onRecv_SaveData((uint8*)buf_temp, &data, ret);
			//readBytes += ret;
		}
		if (ret < 0) break;
	}
	////验证解析出来的真实数据 
	//if (readBytes < 6)
	//{
	//	//LOG_MSG("kcp err...readBytes<0 %d/%d \n", id, readBytes);
	//	return recvBytes;
	//}

	//onRecv_SaveData((uint8*)buf_temp, &data, recvBytes);
	return recvBytes;
}

