
#include "DBManager.h"
#include "GameData.h"
#include "ShareFunction.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace db
{
	//一、线程开始 读取全部数据
	//1、全部账号  启动服务器读取 可以直接保存在内存缓冲中 不用推送 其他读取需要推送到主线程 线程安全
	void DBManager::Thread_BeginAccount()
	{
// 		auto mysql = DBAccount->GetMysqlConnector();
// 
// 		int64_t ftime = clock();
// 		stringstream sql;
// 		sql << "select * from user_account;";
// 		int error = mysql->ExecQuery(sql.str());
// 		if (error != 0)
// 		{
// 			LOG_MSG("[thread:%d] beginAccount error:%d-%s line:%d\n", DBAccount->GetThreadID(), mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
// 			return;
// 		}
// 		int row = mysql->GetQueryRowNum();
// 		for (int i = 0; i < row; i++)
// 		{
// 			int8_t state = 0;
// 			app::S_USER_MEMBER_BASE* mem = new app::S_USER_MEMBER_BASE();
// 			mysql->r("id", mem->ID);
// 			mysql->r("state", state);
// 			mysql->r("username", mem->name);
// 			mysql->r("password", mem->password);
// 			mysql->r("createtime", mem->timeCreate);
// 			mysql->r("logintime", mem->timeLastLogin);
// 			mem->state = (app::E_MEMBER_STATE)state;
// 			LOG_MSG("[thread:%d-%d] name:%s  timecreate:%d  timelogin:%d  \n", DBAccount->GetThreadID(), (int)mem->ID, mem->name, mem->timeCreate, mem->timeLastLogin);
// 
// 
// 			std::string name(mem->name);
// 			app::__AccountsName.insert(std::make_pair(name, mem));
// 			app::__AccountsID.insert(std::make_pair(mem->ID, mem));
// 			mysql->QueryNext();
// 			
// 		}
// 		int64_t value = clock() - ftime;
// 		LOG_MSG("[thread:%d] allmember success...[count:%d]  time：%d毫秒 line:%d\n", DBAccount->GetThreadID(), app::__AccountsName.size(),  value, __LINE__);

	}


	//********************************************************************
	//********************************************************************
	//********************************************************************
	//100 玩家登录获取数据
	void   DBManager::Read_UserLogin(DBBuffer* buff, DBConnetor* db)
	{

	}




}