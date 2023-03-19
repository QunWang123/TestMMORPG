
#include "DBManager.h"
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"

#include "INetBase.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <atomic>
#include "AppManager.h"
#include "AppGlobal.h"
#include "CommandData.h"
#include "ShareFunction.h"

using namespace func;

namespace db
{

	DBManager* __DBManager = nullptr;


	DBManager::DBManager()
	{
		__GameDBXML = nullptr;
		__AccountXML = nullptr;
	}
	DBManager::~DBManager()
	{
		if (__GameDBXML != nullptr)  delete __GameDBXML;
		if (__AccountXML != nullptr) delete __AccountXML;

	}
	//初始化数据库
	void DBManager::InitDB()
	{
		//初始化sql库
		mysql_library_init(0, NULL, NULL);

		__poolBuffs.Init(200, 1 * 1024);
		int ret = LoadDBXML("config_db.xml");
		if (ret == -1) return;

		//初始化数据库数据
		auto readfun = std::bind(&DBManager::Thread_UserRead, this, std::placeholders::_1);
		auto writefun = std::bind(&DBManager::Thread_UserWrite, this, std::placeholders::_1);
		auto begin_account = std::bind(&DBManager::Thread_BeginAccount, this);
		//初始化读线程
		int r_id = 1000;
		int w_id = 2000;
		//DBRead.reserve(1);
		DBWrite.reserve(3);
		//for (int i = 0; i < 1; i++)
		//{
		//	auto r = new db::DBConnetor(__AccountXML);
		//	DBRead.push_back(r);

		//	if (i == 0) r->StartRun(r_id++, readfun, begin_account);
		//	else r->StartRun(r_id++, readfun);
		//}
		//初始化写线程
		for (int i = 0; i < 3; i++)
		{
			auto w = new db::DBConnetor(__AccountXML);
			w->StartRun(w_id++, writefun);
			DBWrite.push_back(w);
		}




	}

	//**************************************************************************************
	//**************************************************************************************
	//**************************************************************************************

	//2、工作线程-读数据
	void DBManager::Thread_UserRead(DBBuffer * buff)
	{
		auto db = (DBConnetor*)buff->GetDB();
		if (db == nullptr)
		{
			LOG_MSG("Thread_UserRead is error:\n");
			return;
		}
		int16_t cmd = 0;
		buff->init_r();
		buff->r(cmd);

		switch (cmd)
		{
		case CMD_10:// Read_10(buff, db); 
			break;
		}
	}
	//3、工作线程-写数据
	void DBManager::Thread_UserWrite(DBBuffer * buff)
	{
		int16_t cmd = 0;
		buff->init_r();
		buff->r(cmd);

		auto db = (DBConnetor*)buff->GetDB();
		if (db == nullptr)
		{
			LOG_MSG("Thread_UserWrite error:%d \n", cmd);
			return;
		}

		switch (cmd)
		{
		case CMD_20:// Write_20(buff, db); 
			break;
		}
	}
	//******************************************************************************
	//******************************************************************************
	//消费者 逻辑主线程
	void  DBManager::update()
	{
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
				// __AppLogin->onDBCommand(buff, cmd);
				break;
			}

			__poolBuffs.Push(buff);
		}
	}
	//生产者 推送buffer到主线程
	void  DBManager::PushToMainThread(DBBuffer * buffer)
	{
		if (buffer == nullptr) return;
		__logicBuffs.push(buffer);
	}
	//消费者 获取buff
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
		sprintf(fpath, "%s%s", func::FileExePath, filename);

		if (__GameDBXML == nullptr) __GameDBXML = new S_DBXML();
		if (__AccountXML == nullptr) __AccountXML = new S_DBXML();
		TiXmlDocument xml;
		if (!xml.LoadFile(fpath))
		{
			LOG_MSG("load config_client.xml iserror... \n");
			return -1;
		}


		TiXmlElement* xmlRoot = xml.RootElement();
		if (xmlRoot == NULL)
		{
			LOG_MSG("xmlRoot == NULL... \n");
			return -1;
		}

		//获取子节点信息1  
		TiXmlElement* xmlNode = xmlRoot->FirstChildElement("gamedb");
		__GameDBXML->port = atoi(xmlNode->Attribute("Port"));
		memcpy(__GameDBXML->ip, xmlNode->Attribute("IP"), 20);
		memcpy(__GameDBXML->username, xmlNode->Attribute("UserName"), 20);
		memcpy(__GameDBXML->userpass, xmlNode->Attribute("UserPass"), 20);
		memcpy(__GameDBXML->dbname, xmlNode->Attribute("DBName"), 20);

		xmlNode = xmlRoot->FirstChildElement("accountdb");
		__AccountXML->port = atoi(xmlNode->Attribute("Port"));
		memcpy(__AccountXML->ip, xmlNode->Attribute("IP"), 20);
		memcpy(__AccountXML->username, xmlNode->Attribute("UserName"), 20);
		memcpy(__AccountXML->userpass, xmlNode->Attribute("UserPass"), 20);
		memcpy(__AccountXML->dbname, xmlNode->Attribute("DBName"), 20);

		return 0;

	}
}
