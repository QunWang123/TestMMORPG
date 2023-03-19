#include "TcpClient.h"

#ifndef ___WIN32_

#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif


namespace net
{
	//********************************************************************
	//********************************************************************
	void TcpClient::runThread()
	{
		m_workthread.reset(new std::thread(TcpClient::run, this));
		m_workthread->detach();

	}
	//工作线程
	void TcpClient::run(TcpClient* tcp)
	{
		auto c = tcp->getData();
		auto socketfd = tcp->getSocket();
		//LOG_MESSAGE("run client...%d:%d \n", c->serverID, c->ID);

		int sleep_time = 10 * 1000;
#ifdef ___WIN32_
		sleep_time = 10;
#endif

		while (true)
		{
			//1、尝试连接
			if (c->state == func::C_FREE)
			{
				tcp->onAutoConnect();
				socketfd = tcp->getSocket();
			}
			else if (c->state == func::C_CONNECTTRY)
			{
				tcp->connectServer();
				socketfd = tcp->getSocket();
			}
			if (c->state < func::C_CONNECT)
			{
#ifdef ___WIN32_
				Sleep(sleep_time);
#else
				usleep(sleep_time);
#endif
				continue;
			}

			// select模型逻辑
			// 将所有的socket都装入一个数据结构fd_set当中
			// 通过select()遍历fd_set数组
			// 取出所有有响应的socket装入另外一个数组中（客户端就一个socket）
			// 处理装有响应的socket数组
			// socket响应FD_ISSET

			//2、连接成功 这是个坑 必须每次重新设置超时时间
			struct timeval tv;
			tv.tv_sec = 10;
			tv.tv_usec = 1000;
			fd_set f_read;				// fd_set就是个scoket最大数组
			FD_ZERO(&f_read);			// FD_ZERO是将f_read的当前滑动位置置于0
			FD_SET(socketfd, &f_read);	// FD_SET是将socketfd设置到f_read当前的滑动位置上
										// 如果是select模型做服务端的话就得把所以socket都塞进f_read
			int errcode = select(socketfd + (u32)1, &f_read, NULL, NULL, &tv);
										// f_read后应该是发，但发的主动姓在客户端手里
			if (errcode > 0)
			{
				// socketfd是否在数组里面
				// 如果是服务端，socketfd可以是连接，读，写，但客户端只管读就Ok
				// socketfd是否有数据可以读
				if (FD_ISSET(socketfd, &f_read))
				{
					int ret = tcp->onRecv();
				}
			}
			else if (errcode == 0)
			{
				// 10秒没收到数据，超时了
			}
			else
			{
#ifdef ___WIN32_
				int err = WSAGetLastError();
				switch (err)
				{
				case WSAEINTR:
					break;
				default:
					tcp->disconnectServer(1001, "select -1");
					break;
				}
#else 
				switch (errno)
				{
				case EINTR:
					break;
				default:
					tcp->disconnectServer(1001, "select -1");
					break;
				}
#endif
			}
		}

		LOG_MESSAGE("client thread exit。。。\n");
		return;
	}


}