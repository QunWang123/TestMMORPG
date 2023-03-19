
#include "DBManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "ShareFunction.h"
#include "WorldData.h"
#include "CommandData.h"

namespace db
{

	//**********************************************************************
	void DBManager::Write_20(DBBuffer* buff, DBConnetor* db)
	{
		app::S_LOGIN_ACCOUNT reg;
		buff->r(&reg, sizeof(app::S_LOGIN_ACCOUNT));

		auto start = std::chrono::steady_clock::now();
		auto mysql = db->GetMysqlConnector();
		u16 childcmd = 0;
		stringstream sql;

		sql << "insert account(username,userpass) values('" << reg.name << "','" << reg.password << "');";
		int ret = mysql->ExecQuery(sql.str());
		if (ret != 0)
		{
			LOG_MESSAGE("[thread:%d] register error:%d-%d-%s line:%d\n",
				db->GetThreadID(), ret, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			childcmd = 1099;
		}

		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
		int ftime = duration.count();

		LOG_MESSAGE("[thread:%d] register success... time: %dms line:%d\n",
			db->GetThreadID(), ftime, __LINE__);

		// 推送数据到主线程
		DBBuffer* main_buff = PopPool();
		main_buff->b(CMD_20);
		main_buff->s(childcmd);
		main_buff->s(&reg, sizeof(app::S_LOGIN_ACCOUNT));
		main_buff->e();
		PushToMainThread(main_buff);
		mysql->CloseQuery();

	}
	//*******************************************************************
	//*******************************************************************

}