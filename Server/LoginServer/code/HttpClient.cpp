#include "HttpClient.h"

namespace http
{
	std::string serverip = "127.0.0.1";
	//std::string serverip = "192.168.32.129";
	int serverport = 8080;

	http::HttpClient::HttpClient()
	{
		InitSocket();
	}
	HttpClient::~HttpClient()
	{
	}

	void HttpClient::Init()
	{
		connectCount = 0;
		socketfd = -1;
		state = EC_FREE;
		m_Request->Reset();
		m_Response->Reset();
	}

	void HttpClient::InitSocket()
	{
		m_Request  = new S_HTTP_BASE();
		m_Response = new S_HTTP_BASE();

		Init();

		WSADATA  wsData;
		int errorcode = WSAStartup(MAKEWORD(2, 2), &wsData);
		if (errorcode != 0) return;

		//运行线程
		m_Thread.reset(new std::thread(HttpClient::run, this));
		m_Thread->detach();
	}
	void HttpClient::ConnectServer()
	{
		if (state == EC_CONNECT) return;
		if (socketfd != -1) closesocket(socketfd);
		socketfd = socket(AF_INET, SOCK_STREAM, 0);

		struct sockaddr_in addrServer;
		addrServer.sin_addr.S_un.S_addr = inet_addr(serverip.c_str());
		addrServer.sin_family = AF_INET;
		addrServer.sin_port = htons(serverport);

		connectCount++;
		int error = connect(socketfd, (struct sockaddr*) & addrServer, sizeof(addrServer));
		if (error == -1)
		{
			LOG_MSG("connct faild....%d count:%d\n", id, connectCount);
			if (connectCount >= 3) return;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ConnectServer();
			return;
		}


		connectCount = 0;
		state = EC_CONNECT;
		setNonblockingSocket(socketfd);

		//禁用ngle算法
		int value = 1;
		setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&value), sizeof(value));
	}

	//工作线程
	void HttpClient::run(HttpClient* c)
	{
		Sleep(5);		// 这里sleep5是因为不这样做在打印的时候会出现乱码（实际无影响）
		LOG_MSG("run HttpThread...%d, socket:%d \n", c->id, c->socketfd);
		while (true)
		{
			S_TEST_BASE* data;
			{
				std::unique_lock<std::mutex> guard(c->m_Mutex);
				while (c->m_HttpDatas.empty())
				{
					//LOG_MSG("************************ thread wait....\n");
					c->m_Condition.wait(guard);
				}
				//LOG_MSG("************************ thread awake....\n");
				data = c->m_HttpDatas.front();
				c->m_HttpDatas.pop_front();
			}

			//先连接
			if (c->state != EC_CONNECT)
			{
				c->ConnectServer();
			}
			//生产数据 写请求数据
			//运行socket
			c->writeData(data);
			c->runSocket();
			c->Init();

			c->pushPool(data);
		}

		LOG_MSG("exit HttpClient 工作线程...%d \n",c->id);
	}

	void HttpClient::runSocket()
	{
		while (true)
		{
			//0、判断有无可读数据
			int err = select_isread(this->socketfd, 0, 5000);
			if(err < 0)
			{
				LOG_MSG("**********************closesocket select_isread....%d-%d, line:%d\n", id, err, __LINE__);
				break;
			}
			//1、接包 粘包处理
			if (err > 0)
			{
				//读取数据
				err = this->recvSocket();
				if (err < 0)
				{
					LOG_MSG("**********************closesocket recvSocket....%d-%d, line:%d\n", id, err, __LINE__);
					break;
				}
			}
			//2、解包
			if (m_Response->state <= ER_HEAD)
			{
				analyData();
			}
			//返回的是个错误
			if (m_Response->state == ER_ERROR)
			{
				LOG_MSG("**********************closesocket analy error....%d-%d\n", id, err);
				break;
			}
			//3、封包 写数据
			{
				std::unique_lock<std::mutex> guard(this->m_Mutex);
				if (!this->m_HttpDatas.empty())
				{
					S_TEST_BASE* data = this->m_HttpDatas.front();
					this->m_HttpDatas.pop_front();
					this->writeData(data);
					//添加回收池
					this->pushPool(data);
				}
			}
			//4、发包
			err = this->sendSocket();
			if (err < 0)
			{
				LOG_MSG("**********************closesocket sendsocket....%d-%d\n", id, err);
				break;
			}
		}
	}

}

