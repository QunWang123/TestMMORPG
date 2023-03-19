
#include "DBManager.h"
#include "tinyxml\tinyxml.h"
#include "tinyxml\tinystr.h"

#include "INetBase.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <atomic>
#include "AppManager.h"
#include "AppGlobal.h"
#include "ShareFunction.h"
#include "CommandData.h"
#include "AppLogin.h"
using namespace func;
using namespace app;


#define  MAX_MYSQL_THREAD_NUM  3
namespace db
{
	
	DBManager* __DBManager = nullptr;
	

	DBManager::DBManager()
	{
		// DBAccount = nullptr;
		__GameDBXML = nullptr;
		__AccountXML = nullptr;
	}
	DBManager::~DBManager()
	{
		if (__GameDBXML != nullptr)  delete __GameDBXML;
		if (__AccountXML != nullptr) delete __AccountXML;

	}
	//��ʼ�����ݿ�
	void DBManager::InitDB()
	{
		__poolBuffs.Init(200,1 * 1024);
		int ret = LoadDBXML("config_db.xml");
		if (ret == -1) return;

		//��ʼ�����ݿ�����
		auto readfun = std::bind(&DBManager::Thread_UserRead, this, std::placeholders::_1);
		auto writefun = std::bind(&DBManager::Thread_UserWrite, this, std::placeholders::_1);
		auto begin_account = std::bind(&DBManager::Thread_BeginAccount, this);
		//��ʼ�����߳�
		int r_id = 1000;
		int w_id = 2000;
		// DBRead.reserve(MAX_MYSQL_THREAD_NUM);
		// DBWrite.reserve(MAX_MYSQL_THREAD_NUM);
		DBRead.reserve(1);
		DBWrite.reserve(1);
		for (int i = 0; i < 1; i++)
		{
			auto r = new db::DBConnetor(__AccountXML);
			DBRead.push_back(r);

			if (i == 0) r->StartRun(r_id++, readfun, begin_account);
			else r->StartRun(r_id++, readfun);
			// r->StartRun(r_id++, readfun);
		}
		//��ʼ��д�߳�
		for (int i = 0; i < 1; i++)
		{
			auto w = new db::DBConnetor(__AccountXML);
			w->StartRun(w_id++,  writefun);
			DBWrite.push_back(w);
		}

		//��ʼ��sql��
		mysql_library_init(0, NULL, NULL);
	}

	//**************************************************************************************
	//**************************************************************************************
	//**************************************************************************************

	//2�������߳�-������
	void DBManager::Thread_UserRead(DBBuffer * buff)
	{
		auto db = (DBConnetor*)buff->GetDB();
		if (db == nullptr)
		{
			LOG_MESSAGE("Thread_UserRead is error:\n");
			return;
		}
		int16_t cmd = 0;
		buff->init_r();
		buff->r(cmd);

		switch (cmd)
		{
		    case CMD_10:Read_10(buff, db); break;
		}
	}
	//3�������߳�-д����
	void DBManager::Thread_UserWrite(DBBuffer * buff)
	{
		int16_t cmd = 0;
		buff->init_r();
		buff->r(cmd);

		auto db = (DBConnetor*)buff->GetDB();
		if (db == nullptr)
		{
			LOG_MESSAGE("Thread_UserWrite error:%d \n", cmd);
			return;
		}

		switch (cmd)
		{
		case CMD_20:Write_20(buff, db); break;
		}
	}

	

	//******************************************************************************
	//******************************************************************************
	//������ �߼����߳�
	void  DBManager::update()
	{
		// ���������ݿ��߳����͵����̵߳�����
		while (!__logicBuffs.empty())
		{
			DBBuffer* buff = nullptr;
			__logicBuffs.try_pop(buff);
			if (buff == nullptr) break;

			u16 cmd;
			buff->init_r();
			buff->r(cmd);

			switch (cmd)
			{
			case CMD_10:
			case CMD_20:
				__AppLogin->onDBCommand(buff, cmd);
				break;
			}
			
			__poolBuffs.Push(buff);
		}
	}
	//������ ����buffer�����߳�
	void  DBManager::PushToMainThread(DBBuffer * buffer)
	{
		if (buffer == nullptr) return;
		__logicBuffs.push(buffer);
	}
	//������ ��ȡbuff
	DBBuffer *DBManager::PopPool()
	{
		auto buff = __poolBuffs.Pop();
		buff->Clear();
		buff->SetDB(nullptr);
		return buff;
	}

	//*******************************************************************************
	//*******************************************************************************
	db::DBConnetor * DBManager::GetDBSource(int type)
	{
		if (type == ETT_USERREAD)
		{
			auto it = DBRead.begin();
			db::DBConnetor* db = *it;
			int value = db->GetWorkCount();
			if (value == 0) return db;

			for (; it != DBRead.end(); ++it)
			{
				db::DBConnetor* pdb = *it;
				if (pdb->GetWorkCount() < db->GetWorkCount())
				{
					db = pdb;
				}
			}
			return db;
		}
		auto it = DBWrite.begin();
		db::DBConnetor* db = *it;
		int value = db->GetWorkCount();
		if (value == 0) return db;

		for (; it != DBWrite.end(); ++it)
		{
			db::DBConnetor* pdb = *it;
			if (pdb->GetWorkCount() < db->GetWorkCount())
			{
				db = pdb;
			}
		}
		return db;
	}

	int DBManager::LoadDBXML(const char * filename)
	{
		char fpath[MAX_FILENAME_LEN];
		memset(fpath, 0, MAX_FILENAME_LEN);
		sprintf_s(fpath, "%s%s", func::FileExePath, filename);

		if (__GameDBXML == nullptr) __GameDBXML = new S_DBXML();
		if (__AccountXML == nullptr) __AccountXML = new S_DBXML();
		TiXmlDocument xml;
		if (!xml.LoadFile(fpath))
		{
			LOG_MESSAGE("load config_client.xml iserror... \n");
			return -1;
		}

	
		TiXmlElement* xmlRoot = xml.RootElement();
		if (xmlRoot == NULL)
		{
			LOG_MESSAGE("xmlRoot == NULL... \n");
			return -1;
		}

		//��ȡ�ӽڵ���Ϣ1  
		TiXmlElement* xmlNode = xmlRoot->FirstChildElement("gamedb");
		__GameDBXML->port = atoi(xmlNode->Attribute("Port"));
		memcpy_s(__GameDBXML->ip, 20, xmlNode->Attribute("IP"), 20);
		memcpy_s(__GameDBXML->username, 20, xmlNode->Attribute("UserName"), 20);
		memcpy_s(__GameDBXML->userpass, 20, xmlNode->Attribute("UserPass"), 20);
		memcpy_s(__GameDBXML->dbname, 20, xmlNode->Attribute("DBName"), 20);

		xmlNode = xmlRoot->FirstChildElement("accountdb");
		__AccountXML->port = atoi(xmlNode->Attribute("Port"));
		memcpy_s(__AccountXML->ip, 20, xmlNode->Attribute("IP"), 20);
		memcpy_s(__AccountXML->username, 20, xmlNode->Attribute("UserName"), 20);
		memcpy_s(__AccountXML->userpass, 20, xmlNode->Attribute("UserPass"), 20);
		memcpy_s(__AccountXML->dbname, 20, xmlNode->Attribute("DBName"), 20);

		return 0;
	
	}
}
