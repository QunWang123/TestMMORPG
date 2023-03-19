
#include "MySqlConnetor.h"

namespace db
{
	MySqlConnetor::MySqlConnetor()
		:mysql(NULL), 
		myres(NULL),
		m_TotalRows(0), 
		m_TotalFields(0), 
		m_curRowIdx(0),
		m_IsConnect(false)
	{	
	}

	MySqlConnetor::~MySqlConnetor()
	{
		if (mysql) mysql = NULL;
		if (myres) myres = NULL;
	}
	//清除sql
	void MySqlConnetor::Clear()
	{
		m_IsConnect = false;
		if (mysql)
		{
			mysql_close(mysql);
			mysql = NULL;
		}
		if (myres) myres = NULL;
	}
	//清理结果集
	void MySqlConnetor::ClearRes()
	{
		bool ifNeedQueryNext = (myres != nullptr);
		if (myres)
		{
			mysql_free_result(myres);
		}

		if (ifNeedQueryNext)
		{
			// myres != nullptr
			while (mysql_next_result(mysql) == 0)
			{
				myres = mysql_store_result(mysql);
				if (myres)
				{
					mysql_free_result(myres);
				}
			}
		}
		else
		{
			// myres == nullptr
			do
			{
				myres = mysql_store_result(mysql);
				if (myres)
				{
					mysql_free_result(myres);
				}
			} while (mysql_next_result(mysql) == 0);
		}

		myres = NULL;
	}
	//*********************************************************************
	//*********************************************************************
	//*********************************************************************
	//0、关闭 清理数据
	void MySqlConnetor::CloseQuery()
	{
		m_curRowIdx = 0;
		m_FieldsValue.clear();
	}
	//1、连接数据库
	bool MySqlConnetor::ConnectMySql(const char * ip, const char * usename, const char * password, const char * dbname, int port)
	{
		strcpy(m_IP, ip);
		strcpy(m_UserName, usename);
		strcpy(m_UserPassword, password);
		strcpy(m_DBName, dbname);
		m_Port = port;
		 
		Clear();
		if(!ReConnect()) return false;
		
		return true;
	}
	//2、重连数据库
	bool MySqlConnetor::ReConnect()
	{
		if (m_IsConnect)
		{
			m_IsGetErrorCode = true;
			int error = mysql_ping(mysql);
			if (error == 0) return true;
			m_IsGetErrorCode = false;
			return false;
		}
		mysql = mysql_init(NULL);
		if (mysql == NULL)
		{
			return false;
		}
		char value = 1;
		int f_outtime = 5;
		// 如果发现连接丢失，启动或禁止与服务器的自动再连接。
		// 从MySQL 5.0.3开始，默认情况下禁止再连接，这是5.0.13中的新选项，
		// 提供了一种以显式方式设置再连接行为的方法。
		mysql_options(mysql, MYSQL_OPT_RECONNECT, &value);
		// MYSQL_OPT_CONNECT_TIMEOUT,以秒为单位的连接超时。
		mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &f_outtime);
		// 用作默认字符集的字符集的名称
		mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");
		bool isconnect = mysql_real_connect(mysql, m_IP, m_UserName, m_UserPassword, m_DBName, m_Port, NULL, CLIENT_MULTI_STATEMENTS);
		if (!isconnect)
		{
			printf("mysql reconnect error:%s \n", GetErrorStr());
			m_IsConnect = false;
			m_IsGetErrorCode = false;
			return false;
		}
		mysql_options(mysql, MYSQL_OPT_RECONNECT, &value);
		m_IsConnect = true;
		m_IsGetErrorCode = true;
		return isconnect;
	}
	//3、执行查询
	int MySqlConnetor::ExecQuery(const std::string & pData)
	{
		if (pData.length() < 1) return -1;
		// 清空结果集
		ClearRes();
		// 看看是不是正常连接
		if(!ReConnect()) return -2;
		// 执行由query指向的SQL查询，它应该是一个length个字节的字符串。
		// 查询必须由一个单个的SQL语句组成。你不应该在语句后增加一个终止的分号(“;”)或\g。
		// 如果查询成功，零。如果发生一个错误，非零。
		int  nRet = mysql_real_query(mysql, pData.c_str(), pData.length());
		if (nRet != 0)
		{
			return nRet;
		}

		// select需要对对表项进行进一步 的处理
		// 非select，比如update xx 或者 insert xx 或者delete from xx，处理完就完了
		// select还要返回来进行修改（读取到内存）之类的
		if ((pData.find("select") == -1) && (pData.find("SELECT") == -1)) return nRet;

		// 1、mysql_store_result(&mysql)告诉句柄mysql，把查询的数据从服务器端取到客户端，然后缓存起来，
		// 放在句柄mysql里面；
		// 2、mysql_store_result()将查询的全部结果读取到客户端，分配1个MYSQL_RES结构，并将结果置于该结构中。
		// （获得全部的查询结果）

		// 对于成功检索了数据的每个查询（SELECT、SHOW、DESCRIBE、EXPLAIN、CHECK TABLE等），
		// 必须调用mysql_store_result()或mysql_use_result() 。
		myres = mysql_store_result(mysql);
		if (myres == nullptr)
		{
			LOG_MESSAGE("mysql ExecQuery error...%s\n",pData.c_str());
			return -1;
		}
		// 返回结果集中的行数,m_TotalFields其实并没有用到。。
		m_TotalFields = mysql_num_fields(myres);
		// 对于SELECT操作，mysql_affected_rows()等价于mysql_num_rows()，即查询结果的行数，
		// // 但是显示使用mysql_num_rows()更加合适。
		// 因此mysql_affected_rows()一般用来在
		// DELETE, INSERT, REPLACE, UPDATE语句执行完成之后判断数据表中变化的行数（如果数据表没有变化，则行数为0）。
		m_TotalRows = static_cast<int>(mysql_affected_rows(mysql));

		GetQueryFields();
		return nRet;
	}
	//4、将通过select等读取的操作的第一行读取出
	// 同时确认共有几个字段，以及每一个字符段的名字（通过filedVec）， 以方便QueryNext();
	// 每一行的数据都以<字段, 数据>的形式被读到m_FieldsValue中
	void MySqlConnetor::GetQueryFields()
	{
		filedVec.clear();
		m_FieldsValue.clear();
		MYSQL_FIELD* filed = NULL;

		// 通过 mysql_fetch_field()获得字段(即表头之类的名称)，
		// mysql_fetch_row()获得每一行的内容(即数值之类的)
		MYSQL_ROW curRow = mysql_fetch_row(myres);
		if (!curRow) return;

		int i = 0;
		while (filed = mysql_fetch_field(myres))
		{
			auto tempRes = curRow[i];
			if (tempRes)
			{
				std::string filedStr(filed->name, filed->name_length);
				m_FieldsValue[filedStr] = tempRes;
				filedVec.push_back(filedStr);
			}
			++i;
		}
	}
	//5、查询下一行的数据
	int MySqlConnetor::QueryNext()
	{
		m_FieldsValue.clear();
		if (myres == nullptr)
		{
			LOG_MESSAGE("mysql QueryNext error...\n");
			return -1;
		}
		
		MYSQL_ROW curRow = mysql_fetch_row(myres);
		if (!curRow)
		{
			return -1;
		}
		int size = filedVec.size();
		for (int i = 0; i < size; ++i)
		{
			auto tempRes = curRow[i];
			if (tempRes)
			{
				m_FieldsValue[filedVec[i]] = tempRes;
			}
		}
		return 0;
	}
	
	//*****************************************************************
	void	MySqlConnetor::r(const char *fieldname, uint8_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (uint8_t)atoi(iter->second.data());
		}
	}
	void	MySqlConnetor::r(const char *fieldname, int8_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (int8_t)atoi(iter->second.data());
		}
	}
	void	MySqlConnetor::r(const char *fieldname, uint16_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (uint16_t)atoi(iter->second.data());
		}
	}
	void	MySqlConnetor::r(const char *fieldname, int16_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (int16_t)atoi(iter->second.data());
		}
	}
	void	MySqlConnetor::r(const char *fieldname, uint32_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (uint32_t)atoi(iter->second.data());
		}
	}
	void MySqlConnetor::r(const char * fieldname, int32_t & value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (int32_t)atoi(iter->second.data());
		}
	}
	void MySqlConnetor::r(const char * fieldname, int64_t & value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end()) 
		{
			// value = _atoi64(iter->second.data());
			value = strtoll(iter->second.data(), NULL, 10);
		}
	}
	void MySqlConnetor::r(const char * fieldname, uint64_t & value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			// value = _atoi64(iter->second.data());
			value = strtoll(iter->second.data(), NULL, 10);
		}
	}
	void MySqlConnetor::r(const char * fieldname, char * value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end()) 
		{
			strcpy(value, iter->second.data());
		}
	}
	void MySqlConnetor::r(const char * fieldname, std::string & value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = iter->second.data();
		}
	}
}

