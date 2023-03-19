
#include "DBManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "ShareFunction.h"
#include "UserManager.h"
#include <chrono>


namespace db
{
	// 更新登录时间
	void DBManager::Write_30(DBBuffer* _buff, DBConnetor* db)
	{
		s64 memid = 0;
		_buff->r(memid);

		int logintime = (int)time(NULL);

		stringstream sql;
		sql << "update user_member set logintime = " << logintime
			<< " where id = " << memid << ";";

		auto start = std::chrono::steady_clock::now();
		auto mysql = db->GetMysqlConnector();
		int err = mysql->ExecQuery(sql.str());

		if (err != 0)
		{
			LOG_MESSAGE("cmd_write_30 failed:%d-%d-%s line:%d\n",
				db->GetThreadID(), err, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			return;
		}

		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
		int ftime = duration.count();
		LOG_MESSAGE("update logintime success[%dms] \n", ftime);
	}


	//**********************************************************************
	// 100 自动注册一个玩家账号以及基础	角色数据信息
	void DBManager::Write_100(DBBuffer* _buff, DBConnetor* db)
	{
		s32 connectid;
		s32 clientid;
		app::S_LOGIN_GAME login;

		_buff->r(connectid);
		_buff->r(clientid);
		_buff->r(&login, sizeof(app::S_LOGIN_GAME));

		auto start = std::chrono::steady_clock::now();
		int curtime = (int)time(NULL);
		// 设置区域ID和表ID
		s32 areaid = func::__ServerInfo->ID;			// 当前服务器配置ID
		s32	tableid = 0;
		s32 length = app::__UserManager->__MemberIDs.size();

		if (length < 200000) tableid = 0;
		else if (length < 400000) tableid = 1;
		else tableid = 2;

		stringstream sql;
		sql << "insert user_member(username, createtime, logintime, areaid, tableid) values("
			<< "'" << login.name << "'," << curtime << "," << curtime << "," << areaid << "," << tableid << ");";

		u16 childcmd = 0;
		auto mysql = db->GetMysqlConnector();
		int err = mysql->ExecQuery(sql.str());

		if (err != 0)
		{
			LOG_MESSAGE("[thread:%d] cmd:100 register error:%d-%d-%s line:%d\n",
				db->GetThreadID(), err, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			childcmd = 1099;
		}
		s64 memid = mysql->mysql->insert_id;

		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
		int ftime = duration.count();
		// 		LOG_MESSAGE("[thread:%d] Write_100  success...time:%dms line:%d\n",
		// 			db->GetThreadID(), ftime, __LINE__);

		DBBuffer* main_buff = PopPool();
		main_buff->b(CMD_100);
		main_buff->s(childcmd);
		main_buff->s(connectid);
		main_buff->s(clientid);
		main_buff->s(memid);
		main_buff->s(tableid);
		main_buff->s(&login, sizeof(app::S_LOGIN_GAME));
		main_buff->e();
		PushToMainThread(main_buff);
	}

	//账号数据库
	//200 更新用户登录时间
	void  DBManager::Write_UserLoginTime(DBBuffer* buff, DBConnetor* db)
	{

	}
	//100 账号注册
	void  DBManager::Write_UserRegister(DBBuffer* _buff, DBConnetor* db)
	{

	}

	std::string getSkillStr(app::S_USER_ONLINE* user)
	{
		std::string str = "";
		for (int i = 0; i < MAX_SKILL_COUNT; i++)
		{
			auto skill = &user->role.stand.myskill.skill[i];
			if (i == 0)
				str = std::to_string(skill->id) + "," +
				std::to_string(skill->level) + "," +
				std::to_string(skill->quick_index);
			else
				str += ";" + std::to_string(skill->id) + "," +
				std::to_string(skill->level) + "," +
				std::to_string(skill->quick_index);
		}
		return str;
	}

	std::string getAtkStr(app::S_USER_ONLINE* user)
	{
		auto atk = &user->role.stand.atk;
		std::string str = std::to_string(atk->hp) + ";" +
			std::to_string(atk->mp) + ";" +
			std::to_string(atk->p_atk)+ ";" +
			std::to_string(atk->m_atk) + ";" +
			std::to_string(atk->p_defend) + ";" +
			std::to_string(atk->m_defend) + ";" +
			std::to_string(atk->dodge) + ";" +
			std::to_string(atk->crit) + ";" +
			std::to_string(atk->dechp) + ";" +
			std::to_string(atk->speed);
		return str;
	}
	// 因为存入数据库会默认少一个\（好像'和"也会少一个。。所以有了这么个函数）
	void replace_all(std::string& src, const std::string& old_value, const std::string& new_value)
	{
		// npos可以表示string的结束位子，是string::type_size 类型的，也就是find（）返回的类型。
		// find函数在找不到指定值得情况下会返回string::npos。
		// 每次重新定位起始位置，防止上轮替换后的字符串形成新的old_value
	// 	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) 
	// 	{
	// 		int tt = string::npos;
	// 		if ((pos = src.find(old_value, pos)) != string::npos)
	// 		{
	// 			src.replace(pos, old_value.length(), new_value);
	// 		}
	// 		else break;
	// 	}
		// 和上面的是等价的
		// string::npos代表string的最后，这还挺方便的
		for (int pos = 0; pos != std::string::npos; pos += new_value.length())
		{
			if ((pos = src.find(old_value, pos)) != std::string::npos)
			{
				src.replace(pos, old_value.length(), new_value);
			}
			else break;
		}
	}


	//玩家离开 保存数据
	void DBManager::Write_RoleData(DBBuffer* _buff, DBConnetor* db)
	{
		u16 cmd;
		s32 userindex;
		s64 memid;
		_buff->r(cmd);
		_buff->r(userindex);
		_buff->r(memid);

		auto user = app::__UserManager->findUser(userindex, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("[thread:%d] user == NULL  %d-%lld\n",
				db->GetThreadID(), userindex, memid);
			return;
		}

		u8 tableid = user->mem.tableID;

		auto start = std::chrono::steady_clock::now();
		std::string tablename;

		if (tableid == 0) tablename = "user_role_0";
		else if (tableid == 1) tablename = "user_role_1";
		else if (tableid == 2) tablename = "user_role_2";

		std::string pos = std::to_string(user->role.base.status.pos.x) + ";" +
			std::to_string(user->role.base.status.pos.y) + ";" +
			std::to_string(user->role.base.status.pos.z);

		std::string c_pos = std::to_string(user->role.base.status.c_pos.x) + ";" +
			std::to_string(user->role.base.status.c_pos.y) + ";" +
			std::to_string(user->role.base.status.c_pos.z);

		u32 level = user->role.base.exp.level;
		u32 bagcount = user->role.stand.bag.num;

		std::string skill = getSkillStr(user);
		std::string atk = getAtkStr(user);
		std::string task = "0";
		std::string combat = serializeString(&user->role.stand.combat);
		std::string bag = serializeString(&user->role.stand.bag);

		replace_all(combat, "\\", "\\\\");
		replace_all(bag, "\\", "\\\\");

		stringstream sql;
		sql << "update " << tablename << " set "
			<< "level = " << level
			<< ",curexp = " << user->role.base.exp.currexp
			<< ",gold = " << user->role.base.econ.gold
			<< ",diamonds = " << user->role.base.econ.diamonds
			<< ",face = " << user->role.base.status.face
			<< ",mapid = " << user->role.base.status.mapid
			<< ",pos = '" << pos
			<< "',c_mapid = " << user->role.base.status.c_mapid
			<< ",c_pos = '" << c_pos
			<< "',skill = '" << skill
			<< "',task = '" << task
			<< "',combat = '" << combat
			<< "',bag_count = " << bagcount
			<< ",bag = '" << bag
			<< "',atk = '" << atk
			<< "' where id = " << user->role.base.innate.id
			<< " and memid = " << memid << ";";

		u16 childcmd = 0;
		auto mysql = db->GetMysqlConnector();
		int err = mysql->ExecQuery(sql.str());
		
		if (err != 0)
		{
			LOG_MESSAGE("[thread:%d] cmd:600 error:%d-%d-%s line:%d\n",
				db->GetThreadID(), err, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			childcmd = 1099;
		}

		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);

		int ftime = duration.count();

		LOG_MESSAGE("[thread:%d] cmd:600 Write_RoleData... %d-%lld [%d]  time:%dms line:%d\n",
			db->GetThreadID(), userindex, memid, cmd, ftime, __LINE__);
		if (cmd == 0) return;		// 是自动保存，直接退出

		DBBuffer* main_buff = PopPool();
		main_buff->b(CMD_600);
		main_buff->s(childcmd);
		main_buff->s(userindex);
		main_buff->s(memid);
		main_buff->e();
		PushToMainThread(main_buff);
	}
	//*******************************************************************
	//*******************************************************************
	// 610 保存基础数据
	void DBManager::Write_RoleBase(DBBuffer* _buff, DBConnetor* db)
	{
		s32 userindex;
		s64 memid;
		_buff->r(userindex);
		_buff->r(memid);

		auto user = app::__UserManager->findUser(userindex, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("[thread:%d] user == NULL  %d-%lld\n",
				db->GetThreadID(), userindex, memid);
			return;
		}

		u8 tableid = user->mem.tableID;

		auto start = std::chrono::steady_clock::now();
		std::string tablename;

		if (tableid == 0) tablename = "user_role_0";
		else if (tableid == 1) tablename = "user_role_1";
		else if (tableid == 2) tablename = "user_role_2";

		std::string pos = std::to_string(user->role.base.status.pos.x) + ";" +
			std::to_string(user->role.base.status.pos.y) + ";" +
			std::to_string(user->role.base.status.pos.z);

		std::string c_pos = std::to_string(user->role.base.status.c_pos.x) + ";" +
			std::to_string(user->role.base.status.c_pos.y) + ";" +
			std::to_string(user->role.base.status.c_pos.z);

		std::string skill = getSkillStr(user);
		std::string atk = getAtkStr(user);

		u32 level = user->role.base.exp.level;
		u32 bagcount = user->role.stand.bag.num;

		stringstream sql;
		sql << "update " << tablename << " set "
			<< "level = " << level
			<< ",curexp = " << user->role.base.exp.currexp
			<< ",gold = " << user->role.base.econ.gold
			<< ",diamonds = " << user->role.base.econ.diamonds
			<< ",face = " << user->role.base.status.face
			<< ",mapid = " << user->role.base.status.mapid
			<< ",pos = '" << pos
			<< "',c_mapid = " << user->role.base.status.c_mapid
			<< ",c_pos = '" << c_pos
			<< "',skill = '" << skill
			<< "',bag_count = " << bagcount
			<< ",atk = '" << atk
			<< "' where id = " << user->role.base.innate.id
			<< " and memid = " << memid << ";";

		u16 childcmd = 0;
		auto mysql = db->GetMysqlConnector();
		int err = mysql->ExecQuery(sql.str());
		if (err != 0)
		{
			LOG_MESSAGE("[thread:%d] cmd:610 error:%d-%d-%s line:%d\n",
				db->GetThreadID(), err, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			childcmd = 1099;
		}

		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);

		int ftime = duration.count();

		LOG_MESSAGE("[thread:%d] cmd:610 Write_RoleBase... %d-%lld time:%dms childcmd: line:%d\n",
			db->GetThreadID(), userindex, memid, ftime, childcmd, __LINE__);
	}

	void DBManager::Write_300(DBBuffer* _buff, DBConnetor* db)
	{
		s32 connectid;
		s32 clientid;
		u8 tableid;
		app::S_CREATE_ROLE role;

		_buff->r(connectid);
		_buff->r(clientid);
		_buff->r(tableid);
		_buff->r(&role, sizeof(app::S_CREATE_ROLE));

		u16 childcmd = 0;

		auto start = std::chrono::steady_clock::now();

		auto mysql = db->GetMysqlConnector();
		stringstream sql;
		std::string tablename;

		if (tableid == 0) tablename = "user_role_0";
		else if (tableid == 1) tablename = "user_role_1";
		else if (tableid == 2) tablename = "user_role_2";

		int job = role.job;
		int sex = role.sex;
		
		sql << "insert " << tablename << "(memid,job,sex,nick,level,mapid,combat,bag,warehouse) values("
			<< role.memid << ","
			<< job << ","
			<< sex << ",'"
			<< role.nick << "',0,0,'""0""','""0""','""0""');";

		int err = mysql->ExecQuery(sql.str());
		if (err != 0)
		{
			LOG_MESSAGE("[thread:%d] cmd:300 error:%d-%d-%s line:%d\n",
				db->GetThreadID(), err, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			childcmd = 1099;
		}
		else
		{
			role.roleid = mysql->mysql->insert_id;
			stringstream sql2;
			std::string sss;
			std::string str_role = "role_0";
			if (role.roleindex == 1) str_role = "role_1";
			else if (role.roleindex == 2) str_role = "role_2";

			sss = std::to_string(role.roleid) + ";" +
				std::to_string(role.job) + ";" +
				std::to_string(role.sex) + ";" +
				"0;0;0;" + role.nick;		// 等级，衣服ID，武器ID，昵称

			sql2 << "update user_member set " << str_role << "='" << sss << "' where id = " << role.memid << ";"
				<< "insert user_nick(nick) values('" << role.nick << "');";
			err = mysql->ExecQuery(sql2.str());
			if (err != 0)
			{
				LOG_MESSAGE("[thread:%d] cmd:300 error:%d-%d-%s line:%d\n",
					db->GetThreadID(), err, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
				childcmd = 1099;
			}
		}

		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
		int ftime = duration.count();
		LOG_MESSAGE("[thread:%d] cmd:300 Write_300... %d-%lld   time:%dms\n",
			db->GetThreadID(), childcmd, role.roleid, ftime);

		DBBuffer* main_buff = PopPool();
		main_buff->b(CMD_300);
		main_buff->s(childcmd);
		main_buff->s(connectid);
		main_buff->s(clientid);
		main_buff->s(&role, sizeof(app::S_CREATE_ROLE));
		main_buff->e();
		PushToMainThread(main_buff);
	}

	void DBManager::Write_400(DBBuffer* _buff, DBConnetor* db)
	{
		s32 connectid;
		s32 clientid;
		u8 tableid;
		app::S_DELETE_ROLE role;

		_buff->r(connectid);
		_buff->r(clientid);
		_buff->r(tableid);
		_buff->r(&role, sizeof(app::S_DELETE_ROLE));

		u16 childcmd = 0;
		auto start = std::chrono::steady_clock::now();
		auto mysql = db->GetMysqlConnector();

		std::string str_role = "role_0";
		if (role.roleindex == 1) str_role = "role_1";
		else if (role.roleindex == 2) str_role = "role_2";

		stringstream sql;
		sql << "update user_member set " << str_role << "='0' where id = " << role.memid << ";";

		int err = mysql->ExecQuery(sql.str());
		if (err != 0)
		{
			LOG_MESSAGE("[thread:%d] cmd:400 error:%d-%d-%s line:%d\n",
				db->GetThreadID(), err, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			childcmd = 1099;
		}
		else
		{
			std::string tablename;
			if (tableid == 0) tablename = "user_role_0";
			else if (tableid == 1) tablename = "user_role_1";
			else if (tableid == 2) tablename = "user_role_2";
			stringstream sql2;

			sql2 << "delete from " << tablename << " where id = " << role.roleid << ";";
			err = mysql->ExecQuery(sql2.str());
			if (err != 0)
			{
				LOG_MESSAGE("[thread:%d] cmd:300 error:%d-%d-%s line:%d\n",
					db->GetThreadID(), err, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
				childcmd = 1099;
			}
		}
		auto current = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
		int ftime = duration.count();
		LOG_MESSAGE("[thread:%d] cmd:400 Write_400... %d-%lld   time:%dms\n",
			db->GetThreadID(), childcmd, role.roleid, ftime);

		DBBuffer* main_buff = PopPool();
		main_buff->b(CMD_400);
		main_buff->s(childcmd);
		main_buff->s(connectid);
		main_buff->s(clientid);
		main_buff->s(&role, sizeof(app::S_DELETE_ROLE));
		main_buff->e();

		PushToMainThread(main_buff);
	}
}