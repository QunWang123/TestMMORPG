#pragma once
#include "IDefine_Http.h"
#include <concurrent_queue.h>
using namespace Concurrency;

namespace http
{
	struct S_TEST_BASE
	{
		char buf[MAX_POST_LENGTH];
		int  len;
		inline void Init()
		{
			memset(this, 0, sizeof(S_TEST_BASE));
		}
	};


	class HttpClient
	{
	public:
		HttpClient();
		~HttpClient();

		concurrent_queue<S_TEST_BASE*>  __PoolBuffs;//安全的并发队列

		int id;//压力测试线程输出的ID
		int connectCount;//连接次数
		std::mutex   m_Mutex;//互斥量 保护数据
		std::condition_variable  m_Condition;//条件变量 用来唤醒线程
		S_HTTP_BASE* m_Request;// 请求数据-发送给服务器的 
		S_HTTP_BASE* m_Response;//响应数据-收到服务器返回数据
		void pushRequest(std::string method, std::string url, int type, const char* c, const int len);
		void writeData(S_TEST_BASE* data);
	private:
		int state;//客户端连接状态
		int socketfd;//客户端创建的socketid
		std::shared_ptr<std::thread>    m_Thread;//线程
		std::list<S_TEST_BASE*>  m_HttpDatas;//发送请求的数据列表

		void Init();
		void InitSocket();
		void ConnectServer();
		void runSocket();
		int sendSocket();
		int recvSocket();
		int analyData();
		int readBody();
		static void run(HttpClient* c);

		inline  void pushPool(S_TEST_BASE* d)
		{
			if (d == nullptr) return;
			__PoolBuffs.push(d);
		}
		S_TEST_BASE* popPool()
		{
			S_TEST_BASE* d = nullptr;
			if (__PoolBuffs.empty() == true)
			{
				d = new S_TEST_BASE();
				return d;
			}

			__PoolBuffs.try_pop(d);
			if (d == nullptr)
			{
				d = new S_TEST_BASE();
				return d;
			}
			return d;
		}
	};

	extern std::string serverip;
	extern int serverport;

}