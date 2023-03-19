#pragma once
#include "IDefine_Http.h"
#include "MySqlConnetor.h"
namespace http
{
	class HttpServer
	{
	private:
		int             m_ConnectCount; //��������
		Socket          m_Listenfd; //����socket
		std::mutex      m_Mutex;//������
		std::mutex      m_ConnectMutex;//���ӻ�����
		std::list<int>  m_Socketfds;//�µ���������
		std::condition_variable         m_Condition;//��������
		std::shared_ptr<std::thread>    m_Thread[MAX_THREAD_COUNT];

		S_HTTP_BASE* m_Request[MAX_THREAD_COUNT];
		S_HTTP_BASE* m_Response[MAX_THREAD_COUNT];
		db::MySqlConnetor*   m_MySql[MAX_THREAD_COUNT];

		int  recvSocket(Socket socketfd, S_HTTP_BASE* quest);
		int  analyData(Socket socketfd, S_HTTP_BASE* quest, S_HTTP_BASE* reponse);
		int  readBody(Socket socketfd, S_HTTP_BASE* quest, S_HTTP_BASE* reponse);
	public:
		HttpServer();
		virtual ~HttpServer();

		int InitSocket();
		void runAccept();
		void runServer();
		void runThread();
		void runSocket(Socket socketfd, int tid);

		void writeData(S_HTTP_BASE* quest, S_HTTP_BASE* reponse, const char* body, int size);
		int  sendSocket(Socket socketfd, S_HTTP_BASE* reponse, int tid);

		static void run(HttpServer* s, int id);
		inline db::MySqlConnetor* getMysql(int id)
		{
			if (id < 0 || id >= MAX_THREAD_COUNT) return NULL;
			return m_MySql[id];
		}
	};
}

