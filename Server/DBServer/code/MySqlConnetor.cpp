
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
	//���sql
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
	//��������
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
	//0���ر� ��������
	void MySqlConnetor::CloseQuery()
	{
		m_curRowIdx = 0;
		m_FieldsValue.clear();
	}
	//1���������ݿ�
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
	//2���������ݿ�
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
		// ����������Ӷ�ʧ���������ֹ����������Զ������ӡ�
		// ��MySQL 5.0.3��ʼ��Ĭ������½�ֹ�����ӣ�����5.0.13�е���ѡ�
		// �ṩ��һ������ʽ��ʽ������������Ϊ�ķ�����
		mysql_options(mysql, MYSQL_OPT_RECONNECT, &value);
		// MYSQL_OPT_CONNECT_TIMEOUT,����Ϊ��λ�����ӳ�ʱ��
		mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &f_outtime);
		// ����Ĭ���ַ������ַ���������
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
	//3��ִ�в�ѯ
	int MySqlConnetor::ExecQuery(const std::string & pData)
	{
		if (pData.length() < 1) return -1;
		// ��ս����
		ClearRes();
		// �����ǲ�����������
		if(!ReConnect()) return -2;
		// ִ����queryָ���SQL��ѯ����Ӧ����һ��length���ֽڵ��ַ�����
		// ��ѯ������һ��������SQL�����ɡ��㲻Ӧ������������һ����ֹ�ķֺ�(��;��)��\g��
		// �����ѯ�ɹ����㡣�������һ�����󣬷��㡣
		int  nRet = mysql_real_query(mysql, pData.c_str(), pData.length());
		if (nRet != 0)
		{
			return nRet;
		}

		// select��Ҫ�ԶԱ�����н�һ�� �Ĵ���
		// ��select������update xx ���� insert xx ����delete from xx�������������
		// select��Ҫ�����������޸ģ���ȡ���ڴ棩֮���
		if ((pData.find("select") == -1) && (pData.find("SELECT") == -1)) return nRet;

		// 1��mysql_store_result(&mysql)���߾��mysql���Ѳ�ѯ�����ݴӷ�������ȡ���ͻ��ˣ�Ȼ�󻺴�������
		// ���ھ��mysql���棻
		// 2��mysql_store_result()����ѯ��ȫ�������ȡ���ͻ��ˣ�����1��MYSQL_RES�ṹ������������ڸýṹ�С�
		// �����ȫ���Ĳ�ѯ�����

		// ���ڳɹ����������ݵ�ÿ����ѯ��SELECT��SHOW��DESCRIBE��EXPLAIN��CHECK TABLE�ȣ���
		// �������mysql_store_result()��mysql_use_result() ��
		myres = mysql_store_result(mysql);
		if (myres == nullptr)
		{
			LOG_MESSAGE("mysql ExecQuery error...%s\n",pData.c_str());
			return -1;
		}
		// ���ؽ�����е�����,m_TotalFields��ʵ��û���õ�����
		m_TotalFields = mysql_num_fields(myres);
		// ����SELECT������mysql_affected_rows()�ȼ���mysql_num_rows()������ѯ�����������
		// // ������ʾʹ��mysql_num_rows()���Ӻ��ʡ�
		// ���mysql_affected_rows()һ��������
		// DELETE, INSERT, REPLACE, UPDATE���ִ�����֮���ж����ݱ��б仯��������������ݱ�û�б仯��������Ϊ0����
		m_TotalRows = static_cast<int>(mysql_affected_rows(mysql));

		GetQueryFields();
		return nRet;
	}
	//4����ͨ��select�ȶ�ȡ�Ĳ����ĵ�һ�ж�ȡ��
	// ͬʱȷ�Ϲ��м����ֶΣ��Լ�ÿһ���ַ��ε����֣�ͨ��filedVec���� �Է���QueryNext();
	// ÿһ�е����ݶ���<�ֶ�, ����>����ʽ������m_FieldsValue��
	void MySqlConnetor::GetQueryFields()
	{
		filedVec.clear();
		m_FieldsValue.clear();
		MYSQL_FIELD* filed = NULL;

		// ͨ�� mysql_fetch_field()����ֶ�(����ͷ֮�������)��
		// mysql_fetch_row()���ÿһ�е�����(����ֵ֮���)
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
	//5����ѯ��һ�е�����
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

