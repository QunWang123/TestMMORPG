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
	//�����߳�
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
			//1����������
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

			// selectģ���߼�
			// �����е�socket��װ��һ�����ݽṹfd_set����
			// ͨ��select()����fd_set����
			// ȡ����������Ӧ��socketװ������һ�������У��ͻ��˾�һ��socket��
			// ����װ����Ӧ��socket����
			// socket��ӦFD_ISSET

			//2�����ӳɹ� ���Ǹ��� ����ÿ���������ó�ʱʱ��
			struct timeval tv;
			tv.tv_sec = 10;
			tv.tv_usec = 1000;
			fd_set f_read;				// fd_set���Ǹ�scoket�������
			FD_ZERO(&f_read);			// FD_ZERO�ǽ�f_read�ĵ�ǰ����λ������0
			FD_SET(socketfd, &f_read);	// FD_SET�ǽ�socketfd���õ�f_read��ǰ�Ļ���λ����
										// �����selectģ��������˵Ļ��͵ð�����socket������f_read
			int errcode = select(socketfd + (u32)1, &f_read, NULL, NULL, &tv);
										// f_read��Ӧ���Ƿ����������������ڿͻ�������
			if (errcode > 0)
			{
				// socketfd�Ƿ�����������
				// ����Ƿ���ˣ�socketfd���������ӣ�����д�����ͻ���ֻ�ܶ���Ok
				// socketfd�Ƿ������ݿ��Զ�
				if (FD_ISSET(socketfd, &f_read))
				{
					int ret = tcp->onRecv();
				}
			}
			else if (errcode == 0)
			{
				// 10��û�յ����ݣ���ʱ��
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

		LOG_MESSAGE("client thread exit������\n");
		return;
	}


}