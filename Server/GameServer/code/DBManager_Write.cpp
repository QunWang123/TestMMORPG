
#include "DBManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "GameData.h"
#include "ShareFunction.h"

namespace db
{

	//**********************************************************************
	//账号数据库
	//200 更新用户登录时间
	void  DBManager::Write_UserLoginTime(DBBuffer* buff, DBConnetor* db)
	{
		
	}
	//100 账号注册
	void  DBManager::Write_UserRegister(DBBuffer* _buff, DBConnetor* db)
	{

	}
	//玩家离开 保存数据
	void DBManager::Write_UserSave(DBBuffer* _buff, DBConnetor* db)
	{

	}
	//*******************************************************************
	//*******************************************************************

}