
#include "TcpServer.h"
#ifdef ___WIN32_
#include "IOPool.h"
using namespace net;


void net::TCPServer::runThread(int num)
{
	m_IsRunning = true;
	m_ThreadNum = num;
	if (num > 10) m_ThreadNum = 10;

	for (int i = 0; i < m_ThreadNum; i++)
		m_workthread[i].reset(new std::thread(TCPServer::run, this, i));

	//分离线程
	for (int i = 0; i < m_ThreadNum; i++)
		m_workthread[i]->detach();
}

void pushContext(IOContext* context)
{
	switch (context->m_Mode)
	{
	case func::SC_WAIT_ACCEPT:
		AcceptContext::push((AcceptContext*)context);
		break;
	case func::SC_WAIT_RECV:
		RecvContext::push((RecvContext*)context);
		break;
	case func::SC_WAIT_SEND:
		SendContext::push((SendContext*)context);
		break;
	}
}
//工作线程
void  net::TCPServer::run(TCPServer* tcp, int id)
{
	LOG_MESSAGE("run workthread...%d\n", id);
	ULONG_PTR    key = 1;//完成端口绑定的字段
	OVERLAPPED*  overlapped = nullptr;//连入Socket的时候建立的那个重叠结构  
	DWORD        recvBytes = 0;//操作完成返回字节数

	while (tcp->m_IsRunning)
	{
		// 不断通过GetQueuedCompletionStatus扫描完成端口中是否有IO操作完成，
		// 有的话，将已经完成的IO操作去除，处理完成后，再投递一个IO请求
		bool iscomplete = GetQueuedCompletionStatus(tcp->getCompletePort(), &recvBytes, &key, &overlapped, INFINITE);

		// 该宏的功能，是根据某个结构体中成员变量的地址，计算出结构体地址。
		// address，成员变量地址
		// type，结构体类型
		// field，成员变量名
		IOContext* context = CONTAINING_RECORD(overlapped, IOContext, m_OverLapped);
		if (context == nullptr) continue;

		if (iscomplete == false)
		{
			DWORD dwErr = GetLastError();
			// 如果是超时了，就再继续等吧  
			if (WAIT_TIMEOUT == dwErr) continue;

			if (overlapped != NULL)
			{
				tcp->shutDown(context->m_Socket, context->m_Mode, NULL, 3001);
				pushContext(context);
				continue;
			}

			tcp->shutDown(context->m_Socket, context->m_Mode, NULL, 3002);
			pushContext(context);
			continue;
		}
		else
		{
			if (overlapped == NULL)
			{
				LOG_MESSAGE("overlapped == NULL \n");
				break;
			}
			if (key != 0)
			{
				LOG_MESSAGE("key != 0 \n");
				continue;
			}

			// 判断是否有客户端断开了
			if ((recvBytes == 0) && (context->m_Mode == func::SC_WAIT_RECV || context->m_Mode == func::SC_WAIT_SEND))
			{
				tcp->shutDown(context->m_Socket, context->m_Mode, NULL, 3003);
				pushContext(context);
				continue;
			}

			switch (context->m_Mode)
			{
			case func::SC_WAIT_ACCEPT:
				{
					auto acc = (AcceptContext*)context;
					int err = tcp->onAccpet(acc);
					if (err != 0)
					{
						tcp->closeSocket(acc->m_Socket, NULL, 3004);
						AcceptContext::push(acc);
						tcp->postAccept();
					}
				}
				break;
			case func::SC_WAIT_RECV:
				tcp->onRecv(context, (int)recvBytes, id);
				break;
			case func::SC_WAIT_SEND:
				tcp->onSend(context, (int)recvBytes);
				break;
			}
		}
	}


	LOG_MESSAGE("exit workthread...%d\n", id);
}

//消费者 发送线程 
//void net::TCPServer::run_send(TCPServer* tcp)
//{
//	//LOG_MESSAGE("run sendthread...\n");
//
//	//while (tcp->m_IsRunning)
//	//{
//	//	for (s32 i = 0; i < func::__ServerInfo->MaxConnect; i++)
//	//	{
//	//		auto c = tcp->client(i);
//	//		if (c == nullptr) continue;
//	//		if (c->ID == -1)   continue;
//	//		if (c->state == func::S_Free) continue;
//	//		if (c->closeState == func::S_CLOSE_SHUTDOWN) continue;
//	//		if (!c->is_SendCompleted) continue;
//
//	//		tcp->postSend(c);
//	//	}
//
//	//	Sleep(10);
//	//}
//
//	//LOG_MESSAGE("exit sendthread...\n");
//}


#endif