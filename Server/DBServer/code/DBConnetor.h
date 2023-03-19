// ������ͨ�������ָ�����ݰ�read��write����DBManager
// DBManager�е�read_xx��write_xx��ͨ�����ｫ���ݿ�ָ���MySqlConnetor���棬�����������ݿ⴦��
// ��ΪDBManager::GetDBSource����������õ�DBConnetor�����Լ�ָ������ҪDBManager�е�Read����write
// 
// ���ͨ��DBManager::GetDBSource�õ���read��write��Ӧ��DBConnetor����ָ��CMD_XX����DBConnetor
// DBConnetor���ݳ�ʼ�󶨵�m_Callback����DBManager�е�Thread_UserWrite����Thread_UserRead����
// DBManager����CMD_XXָ��ִ��Write_xx��Read_xx�������ں�������д��mysql��䣬��DBManager�л�ȡMySqlConnetor mysql
// �Ӷ���mysql��䴫��MySqlConnetor��ȥ
// ע�⣺DBConnetor��ȫ���ж�Ӧ�Ķ���ͬһ����
// ָ���DBManagerʱ������DBBuffer��֪�����ĸ�DBConnetor

#ifndef  ____MYSQLMANAGER_H
#define  ____MYSQLMANAGER_H
#include <string>
#include <functional>
#include <atomic>
#include <queue>
#include <mutex> //������
#include <thread> 

#include "DBBuffer.h"
#include "ThreadSafeQueue.h"
#include "MySqlConnetor.h"



namespace db
{
	typedef std::function<void(DBBuffer*)> Callback;
	typedef std::function<void()> BeginCallback;

	struct S_DBXML
	{
		int	    port;//�˿ں�
		char	ip[20];//IP
		char	username[20];//���ݿ��˺�
		char	userpass[20];//���ݿ�����
		char	dbname[20];//���ݿ�����

		S_DBXML() { memset(this, 0, sizeof(*this)); }
	};
	//�����߳�����
	enum E_THREAD_TYPE
	{
		ETT_USERREAD  = 0x01,
		ETT_USERWRITE  = 0x02,
		ETT_ACCOUNT   = 0x03
	};

	class DBConnetor
	{
	public:
		DBConnetor(S_DBXML* cfg);
		~DBConnetor();
	private:
		MySqlConnetor *mysql;//mysql������
		S_DBXML*	   m_mysqlcfg;//���ݿ����ñ�

		//�߳��������
		int       m_ThreadID;//�߳�ID
		Callback  m_Callback;//�ص�����
		BeginCallback  m_BeginCallback;//�ص�����

		std::mutex	m_Mutex;
		// ���˫������˼·����
		std::queue<DBBuffer*> m_Queue;//�������ݶ��У�Ϊ�˱�֤m_Queue������ȫ(��\ȡ��Ҫ����)�ҿ���
										// ��ȡ����һֱ����.��Ϊ:while(!m_Queue.empty())˵���������ܾ�
										// ��˻Ὣm_Queue������m_SwapQueue�У�ȡ����m_Queue����
										// �ҿ��Է��ĵ�while(!m_SwapQueue.empty())
										// �����Ϣ�ٶ�̫�죬�п���m_SwapQueue��û������ֺ�m_Queue����
										// ��Ϊ���е�����m_SwapQueue��ִ��һϵ���µ�ָ��ֽ�������û����ļ�����
		std::queue<DBBuffer*> m_SwapQueue;//�������ݶ���
		std::condition_variable m_ConditionVar;//��������
		ThreadSafePool  m_BuffsPool;//BUFF��
		atomic<int>  m_WorkCount;

	public:
		int						Stop();
		int						StartRun(int id,  Callback callback, BeginCallback bcallback = nullptr);
		DBBuffer*				PopBuffer();
		void					PushToThread(DBBuffer* buffer);
		inline MySqlConnetor*	GetMysqlConnector() { return mysql; }
		inline int				GetThreadID() { return m_ThreadID; }
		inline int				GetWorkCount()const { return m_WorkCount; }
		void					Run();
	};

	
}


#endif