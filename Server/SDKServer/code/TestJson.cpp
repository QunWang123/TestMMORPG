#include "TestJson.h"
#include "json.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std::chrono;
using Json = nlohmann::json;


namespace json
{

	std::string reponseJson(int cmd,int err, std::string account, std::string des,int id,int socketid)
	{
		Json js;
		js["cmd"] = cmd;
		js["err"] = err;
		js["account"] = account;
		js["des"] = des;
		js["id"] = id;
		js["socketid"] = socketid;
		return js.dump();
	}
	//cmd 1000 
	void json_login(HttpServer* server, S_HTTP_BASE* quest, S_HTTP_BASE* reponse, Json& js,int cmd)
	{
		auto js_id  = js["id"];
		auto js_socketid = js["socketid"];
		auto js_account = js["account"];
		auto js_password = js["password"];
		if (js_id.is_number_integer() == false) return;
		if (js_socketid.is_number_integer() == false) return;
		if (js_account.is_string() == false) return;
		if (js_password.is_string() == false) return;

		int id = js_id;
		int socketid = js_socketid;
		std::string account = js_account;
		std::string password = js_password;

		//账号全部转为小写 密码区分大小写 账号不区分大小写
		std::string account2 = "";
		std::transform(account.begin(), account.end(), std::back_inserter(account2), ::tolower);


		auto start = std::chrono::steady_clock::now();
		auto mysql = server->getMysql(quest->threadid);
		if (mysql == nullptr) return;

		std::stringstream sql;
		sql << "select * from account where username = '" << account2 << "';" ;

		int error = mysql->ExecQuery(sql.str());
		if (error != 0)
		{
			LOG_MSG("[thread:%d] mysql error:%d-%s line:%d\n", quest->threadid, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			
			std::string err = reponseJson(cmd, 1, account,"mysql err...",id,socketid);
			reponse->SetResponseLine(406, "Failed");
			server->writeData(quest, reponse, err.c_str(), err.length());
			return;
		}

		//没有这个账号 需要注册
		int row = mysql->GetQueryRowNum();
		if (row != 1)
		{
			std::string err = reponseJson(cmd, 2, account, "not find account...", id, socketid);
			reponse->SetResponseLine(200, "OK");
			server->writeData(quest, reponse, err.c_str(), err.length());
			return;
		}

		std::string password2 = "";
		mysql->r("userpass", password2);
		mysql->QueryNext();

		auto current = std::chrono::steady_clock::now();
		auto duration = duration_cast<milliseconds>(current - start);
		int ftime = duration.count();

		LOG_MSG("登录：mysql successfully...%d %s time:%d\n", quest->threadid,account2.c_str(), ftime);

		if (strcmp(password.c_str(), password2.c_str()) == 0)
		{
			LOG_MSG("Password verification passed...\n");
			std::string err = reponseJson(cmd, 0, account, "login successfully...", id, socketid);
			reponse->SetResponseLine(200, "OK");
			server->writeData(quest, reponse, err.c_str(), err.length());
			quest->temp_str.clear();
		}
		else
		{
			std::string err = reponseJson(cmd, 3, account,"password err...", id, socketid);
			reponse->SetResponseLine(200, "OK");
			server->writeData(quest, reponse, err.c_str(), err.length());
			quest->temp_str.clear();
		}
	}
	//cmd 2000 
	void json_reg(HttpServer* server, S_HTTP_BASE* quest, S_HTTP_BASE* reponse, Json& js, int cmd)
	{
		auto js_id = js["id"];
		auto js_socketid = js["socketid"];
		auto js_account = js["account"];
		auto js_password = js["password"];
		if (js_id.is_number_integer() == false) return;
		if (js_socketid.is_number_integer() == false) return;
		if (js_account.is_string() == false) return;
		if (js_password.is_string() == false) return;

		int id = js_id;
		int socketid = js_socketid;
		std::string account = js_account;
		std::string password = js_password;

		//账号全部转为小写 密码区分大小写 账号不区分大小写
		std::string account2 = "";
		std::transform(account.begin(), account.end(), std::back_inserter(account2), ::tolower);


		auto start = std::chrono::steady_clock::now();
		auto mysql = server->getMysql(quest->threadid);
		if (mysql == nullptr) return;

		std::stringstream sql;
		sql << "insert into account(username,userpass) values('" << account2 
			<< "','" << password
			<< "');";

		int error = mysql->ExecQuery(sql.str());
		if (error != 0)
		{
			LOG_MSG("[thread:%d] mysql error:%d-%s line:%d\n", quest->threadid, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);

			std::string err = reponseJson(cmd, 1, account,"account already...",id,socketid);
			reponse->SetResponseLine(406, "Failed");
			server->writeData(quest, reponse, err.c_str(), err.length());
			return;
		}

		std::string err = reponseJson(cmd, 0, account,"reg successfully...", id, socketid);
		reponse->SetResponseLine(200, "OK");
		server->writeData(quest, reponse, err.c_str(), err.length());
		quest->temp_str.clear();
	}
	void onCommand(HttpServer* server, S_HTTP_BASE* quest, S_HTTP_BASE* reponse)
	{
		if (strcmp(quest->url.c_str(), "/checksdk") != 0)
		{
			reponse->SetResponseLine(404, "Failed");
			server->writeData(quest, reponse, "url err", 7);
			return;
		}
		quest->temp_str.clear();
		quest->temp_str.assign(&quest->buf[quest->pos_head], quest->Content_length);
		
		quest->pos_head += quest->Content_length;
		quest->state = ER_OVER;

		//不允许抛出错误？
		Json js = Json::parse(quest->temp_str,nullptr,false,false);
		if (js.is_object() == false) 
		{
			reponse->SetResponseLine(404, "Failed");
        	server->writeData(quest, reponse, "not object", 10);
			return;
		}
		auto js_cmd = js["cmd"];
		if (js_cmd.is_number_integer() == false) 
		{
			reponse->SetResponseLine(404, "Failed");
			server->writeData(quest, reponse, "not int", 7);
			return;
		}

		int cmd = js_cmd;


		switch (cmd)
		{
		case 10://登录
			json_login(server, quest, reponse, js, cmd);
			break;
		case 20://注册
			json_reg(server, quest, reponse, js, cmd);
			break;
		}
	}
}

