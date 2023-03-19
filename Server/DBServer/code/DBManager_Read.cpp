
#include "DBManager.h"
#include "ShareFunction.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "UserData.h"
#include <chrono>
#include "UserManager.h"
#include "CommandData.h"


namespace db
{
	void readRoleNick(db::DBConnetor* db)
	{
		auto start = std::chrono::steady_clock::now();
		auto mysql = db->GetMysqlConnector();
		
		stringstream sql;
		sql << "select * from user_nick;";
		int error = mysql->ExecQuery(sql.str());
// 		stringstream sttt;
// 		sttt << "select * from teble where id = xxx";
// 		mysql->ExecQuery(sttt.str());
		/*mysql*/
		if (error != 0)
		{
			LOG_MESSAGE("[thread:%d] readRoleNick error:%d-%s line:%d\n",
				db->GetThreadID(), mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			return;
		}
		int row = mysql->GetQueryRowNum();
		for (int i = 0; i < row; i++)
		{
			std::string nick;

			mysql->r("nick", nick);
			if (nick.size() < 1) continue;
			app::__UserManager->insertNick((char*)nick.c_str());
			// LOG_MESSAGE("nick:%s\n", nick.c_str());
			mysql->QueryNext();
		}
		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
		int ftime = duration.count();
		LOG_MESSAGE("[thread:%d] nick success...[count:%d]  time：%dms line:%d\n",
			db->GetThreadID(), (int)app::__UserManager->__MemberNames.size(), ftime, __LINE__);
	}


	//一、线程开始 读取全部数据
	//1、全部账号  启动服务器读取 可以直接保存在内存缓冲中 不用推送 其他读取需要推送到主线程 线程安全
	void DBManager::Thread_BeginAccount()
	{
		auto start = std::chrono::steady_clock::now();
		auto mysql = DBRead[0]->GetMysqlConnector();

		stringstream sql;
		sql << "select * from user_member;";
		int error = mysql->ExecQuery(sql.str());
		if (error != 0)
		{
			LOG_MESSAGE("[thread:%d] beginAccount error:%d-%s line:%d\n", DBRead[0]->GetThreadID(), mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			return;
		}
		int row = mysql->GetQueryRowNum();
		for (int i = 0; i < row; i++)
		{
			app::S_USER_MEMBER_BASE* m = new app::S_USER_MEMBER_BASE();
			mysql->r("id", m->mem.id);
			mysql->r("state", m->mem.state);
			mysql->r("username", m->mem.name);
			mysql->r("createtime", m->mem.timeCreate);
			mysql->r("logintime", m->mem.timeLastLogin);
			mysql->r("areaid", m->mem.areaID);
			mysql->r("tableid", m->mem.tableID);

			// 解析读取角色基础数据
			for (int index = 0; index < USER_MAX_ROLE; index++)
			{
				std::string s = "role_" + std::to_string(index);
				std::string ss;
				mysql->r(s.c_str(), ss);
				std::vector<std::string> arr = share::split(ss, ";", true);
				if (arr.size() != 7) continue;

				auto role = &m->role[index];
				u64 roleid = strtoll(arr[0].c_str(), NULL, 10);
				if (roleid < 1000) continue;

				role->id = roleid;
				role->job = atoi(arr[1].c_str());
				role->sex = atoi(arr[2].c_str());
				role->level = atoi(arr[3].c_str());
				role->clothid = atoi(arr[4].c_str());
				role->weaponid = atoi(arr[5].c_str());
				memcpy(role->nick, arr[6].c_str(), USER_MAX_NICK);

 				LOG_MESSAGE("roledata:%d-%lld %d-%d-%d %d-%d nick:%s\n",
 					index, role->id, role->job, role->sex, role->level, role->clothid, role->weaponid, role->nick);
			}
			app::__UserManager->insertMember(m);
			mysql->QueryNext();
		}
		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
		int ftime = duration.count();
		LOG_MESSAGE("[thread:%d] allmember success...[count:%d]  time：%dms line:%d\n",
			DBRead[0]->GetThreadID(), (int)app::__UserManager->__MemberNames.size(), ftime, __LINE__);

		// 读取角色昵称
		readRoleNick(DBRead[0]);
	}

	//********************************************************************
	//********************************************************************
	//********************************************************************
	void DBManager::Read_200(DBBuffer* buff, DBConnetor* db)
	{
		s32 connectID;
		s32 clientID;
		u8 tableid;
		s64 roleid;
		app::S_SELECT_ROLE role;
		buff->r(connectID);
		buff->r(clientID);
		buff->r(tableid);
		buff->r(roleid);
		buff->r(&role, sizeof(app::S_SELECT_ROLE));

		u16 childcmd = 0;
		app::S_USER_ROLE roledata;		// 角色数据
		// roledata.reset();

		auto start = std::chrono::steady_clock::now();
		auto mysql = db->GetMysqlConnector();
		
		std::string tablename;
		if (tableid == 0) tablename = "user_role_0";
		else if (tableid == 1) tablename = "user_role_1";
		else if (tableid == 2) tablename = "user_role_2";

		stringstream sql;
		sql << "select * from " << tablename << " where id = " << roleid << " and memid = " << role.memid << ";";
		int err = mysql->ExecQuery(sql.str());
		if (err != 0)
		{
			LOG_MESSAGE("[thread:%d] cmd_read_200 error:%d-%s line:%d\n",
				db->GetThreadID(), mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			childcmd = 1098;
		}

		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
		int ftime = duration.count();

		int row = mysql->GetQueryRowNum();
		if (row != 1)
		{
			if (childcmd == 0) childcmd = 1099;
			DBBuffer* main_buff = PopPool();
			main_buff->b(CMD_200);
			main_buff->s(childcmd);
			main_buff->s(connectID);
			main_buff->s(clientID);
			main_buff->s(&role, sizeof(app::S_SELECT_ROLE));
			main_buff->e();
			PushToMainThread(main_buff);

			LOG_MESSAGE("[thread:%d] cmd_read_200-%d time:%dms\n",
				db->GetThreadID(), childcmd, ftime);
			return;
		}
		else
		{
			deserializeBinary(mysql, &roledata);
		}
		LOG_MESSAGE("[thread:%d] cmd_read_200-%d time:%dms\n",
			db->GetThreadID(), childcmd, ftime);
		DBBuffer* main_buff = PopPool();
		main_buff->b(CMD_200);
		main_buff->s(childcmd);
		main_buff->s(connectID);
		main_buff->s(clientID);
		main_buff->s(&role, sizeof(app::S_SELECT_ROLE));
		main_buff->s(&roledata, sizeof(app::S_USER_ROLE));

		main_buff->e();
		PushToMainThread(main_buff);
	}
}