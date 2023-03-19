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

		concurrent_queue<S_TEST_BASE*>  __PoolBuffs;//��ȫ�Ĳ�������

		int id;//ѹ�������߳������ID
		int connectCount;//���Ӵ���
		std::mutex   m_Mutex;//������ ��������
		std::condition_variable  m_Condition;//�������� ���������߳�
		S_HTTP_BASE* m_Request;// ��������-���͸��������� 
		S_HTTP_BASE* m_Response;//��Ӧ����-�յ���������������
		void pushRequest(std::string method, std::string url, int type, const char* c, const int len);
		void writeData(S_TEST_BASE* data);
	private:
		int state;//�ͻ�������״̬
		int socketfd;//�ͻ��˴�����socketid
		std::shared_ptr<std::thread>    m_Thread;//�߳�
		std::list<S_TEST_BASE*>  m_HttpDatas;//��������������б�

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