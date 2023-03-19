
#include "DBManager.h"
#include "WorldData.h"
#include "ShareFunction.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "AppGlobal.h"
#include "UserManager.h"
#include <chrono>
#include "CommandData.h"


namespace db
{
	//一、线程开始 读取全部数据
	//1、全部账号  启动服务器读取 可以直接保存在内存缓冲中 不用推送 其他读取需要推送到主线程 线程安全
	void DBManager::Thread_BeginAccount()
	{
		auto start = std::chrono::steady_clock::now();
		auto mysql = DBRead[0]->GetMysqlConnector();
		stringstream sql;
		sql << "select * from account";
		int err = mysql->ExecQuery(sql.str());
		if (err != 0)
		{
			LOG_MESSAGE("[thread:%d] beginAccount error:%d-%s line:%d\n", DBRead[0]->GetThreadID(), mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			return;
		}
		int row = mysql->GetQueryRowNum();
		for (int i = 0; i < row; i++)
		{
			app::S_ACCOUNT_BASE* acc = new app::S_ACCOUNT_BASE();
			mysql->r("username", acc->name);
			mysql->r("userpass", acc->password);
			// LOG_MESSAGE("user:%s-%s\n", acc->name, acc->password);
			app::__UserManager->insetAccount(acc);
			mysql->QueryNext();
		}
		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
		int ftime = duration.count();
		LOG_MESSAGE("[thread:%d] account success... [count:%d] time: %dms line:%d\n", 
			DBRead[0]->GetThreadID(), (int)app::__UserManager->__Accounts.size(), ftime, __LINE__);
	}


	//********************************************************************
	//********************************************************************
	//********************************************************************
	//100 玩家登录获取数据
// 	void   DBManager::Read_UserLogin(DBBuffer* buff, DBConnetor* db)
// 	{
// 
// 	}

	// 10 从数据库查询数据
	void DBManager::Read_10(DBBuffer* buff, DBConnetor* db)
	{
		app::S_LOGIN_ACCOUNT login;
		buff->r(&login, sizeof(app::S_LOGIN_ACCOUNT));

		auto start = std::chrono::steady_clock::now();
		auto mysql = db->GetMysqlConnector();
		u16 childcmd = 0;
		stringstream sql;
		sql << "select * from account where username = '" << login.name << "';";
		int error = mysql->ExecQuery(sql.str());
		if (error != 0)
		{
			LOG_MESSAGE("[thread:%d] login error:%d-%s line:%d\n", 
				db->GetThreadID(), mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			childcmd = 1098;
		}
		else
		{
			int row = mysql->GetQueryRowNum();
			if (row == 0)
			{
				// 没有该账号
				childcmd = 1002;
			}
			else
			{
				std::string password;
				mysql->r("password", password);
				if (strcmp(login.password, password.c_str()) != 0)
				{
					// 密码不对
					childcmd = 1003;
				}
			}
		}

		// 推送数据到主线程
		DBBuffer* main_buff = PopPool();
		main_buff->b(CMD_10);
		main_buff->s(childcmd);
		main_buff->s(&login, sizeof(app::S_LOGIN_ACCOUNT));
		main_buff->e();
		PushToMainThread(main_buff);
		mysql->CloseQuery();
	}


}