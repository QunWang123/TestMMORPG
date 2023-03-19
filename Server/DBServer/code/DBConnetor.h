// 外界如果通过这里把指令数据按read和write发到DBManager
// DBManager中的read_xx和write_xx中通过这里将数据库指令发送MySqlConnetor里面，进而发到数据库处理
// 因为DBManager::GetDBSource，发给外界用的DBConnetor可以自己指定是需要DBManager中的Read还是write
// 
// 外界通过DBManager::GetDBSource得到和read和write对应的DBConnetor，将指令CMD_XX传入DBConnetor
// DBConnetor根据初始绑定的m_Callback调用DBManager中的Thread_UserWrite或者Thread_UserRead函数
// DBManager根据CMD_XX指令执行Write_xx和Read_xx函数，在函数里面写好mysql语句，从DBManager中获取MySqlConnetor mysql
// 从而将mysql语句传到MySqlConnetor中去
// 注意：DBConnetor在全程中对应的都是同一个！
// 指令传入DBManager时，根据DBBuffer来知晓是哪个DBConnetor

#ifndef  ____MYSQLMANAGER_H
#define  ____MYSQLMANAGER_H
#include <string>
#include <functional>
#include <atomic>
#include <queue>
#include <mutex> //互斥锁
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
		int	    port;//端口号
		char	ip[20];//IP
		char	username[20];//数据库账号
		char	userpass[20];//数据库密码
		char	dbname[20];//数据库名字

		S_DBXML() { memset(this, 0, sizeof(*this)); }
	};
	//开启线程类型
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
		MySqlConnetor *mysql;//mysql连接器
		S_DBXML*	   m_mysqlcfg;//数据库配置表

		//线程相关数据
		int       m_ThreadID;//线程ID
		Callback  m_Callback;//回调函数
		BeginCallback  m_BeginCallback;//回调函数

		std::mutex	m_Mutex;
		// 这个双队列神思路啊！
		std::queue<DBBuffer*> m_Queue;//共享数据队列，为了保证m_Queue尽量安全(放\取都要上锁)且快速
										// 则取不能一直上锁.因为:while(!m_Queue.empty())说不定会锁很久
										// 因此会将m_Queue交换到m_SwapQueue中，取消了m_Queue的锁
										// 且可以放心的while(!m_SwapQueue.empty())
										// 如果消息速度太快，有可能m_SwapQueue还没跑完就又和m_Queue交换
										// 因为队列的特性m_SwapQueue会执行一系列新的指令，又交换回来没跑完的继续跑
		std::queue<DBBuffer*> m_SwapQueue;//交换数据队列
		std::condition_variable m_ConditionVar;//条件变量
		ThreadSafePool  m_BuffsPool;//BUFF池
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