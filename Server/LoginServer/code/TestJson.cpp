#include "TestJson.h"
#include "json.hpp"
#include "AppGlobal.h"
#include "WorldData.h"
#include "ShareFunction.h"
#include "CommandData.h"
#include "AppLogin.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <concurrent_queue.h>

using namespace app;
using namespace Concurrency;
using namespace std::chrono;
using Json = nlohmann::json;


namespace json
{
	concurrent_queue<std::string>  __Buffs;//安全的并发队列
	void json_login(Json& js);
	void json_reg(Json& js);

	void pushMainData(std::string str)
	{
		__Buffs.push(str);
	}
	void updateJson()
	{
		while (!__Buffs.empty())
		{
			std::string str = "";
			__Buffs.try_pop(str);
			if (str.length() < 1) break;

			//解析json数据
			Json js = Json::parse(str, nullptr, false, false);
			if (js.is_object() == false) return;
			auto js_cmd = js["cmd"];
			if (js_cmd.is_number_integer() == false)  return;
			int cmd = js_cmd;

			switch (cmd)
			{
			case CMD_10://登录
				json_login(js);
				break;
			case CMD_20://注册
				json_reg(js);
				break;
			}
		}
	}

	//cmd 1000 
	void json_login(Json& js)
	{
		auto js_err  = js["err"];
		auto js_account = js["account"];
		auto js_des = js["des"];
		auto js_id = js["id"];
		auto js_socketid = js["socketid"];

		if (js_err.is_number_integer() == false) return;
		if (js_account.is_string() == false) return;
		if (js_des.is_string() == false) return;
		if (js_id.is_number_integer() == false) return;
		if (js_socketid.is_number_integer() == false) return;

		int err = js_err;
		std::string account = js_account;
		std::string des = js_des;
		int id = js_id;
		int socketid = js_socketid;

		char ftime[30];
		share::formatTime(time(0), ftime);


		LOG_MSG("%s 10:%d  %s id:%d/%d\n", ftime, err, des.c_str(), id,socketid);

		auto c = __TcpServer->client(id);
		if (c == nullptr || c->socketfd != socketid || c->state < func::S_CONNECT_SECURE)
		{
			LOG_MSG("json_login 没有找到玩家数据...%d %d\n", id, socketid);
			return;
		}
		if (err != 0)
		{
			sendErrInfo(c->ID, CMD_10, err);
			return;
		}

		app::S_LOGIN_ACCOUNT  login;
		login.reset();
		login.connectid = id;
		sprintf_s(login.name, "%s", account.c_str());

		//发送登录成功
		loginSuccess(c->ID, &login);
	}
	//cmd 20 
	void json_reg(Json& js)
	{
		auto js_err = js["err"];
		auto js_account = js["account"];
		auto js_des = js["des"];
		auto js_id = js["id"];
		auto js_socketid = js["socketid"];

		if (js_err.is_number_integer() == false) return;
		if (js_account.is_string() == false) return;
		if (js_des.is_string() == false) return;
		if (js_id.is_number_integer() == false) return;
		if (js_socketid.is_number_integer() == false) return;

		int err = js_err;
		std::string account = js_account;
		std::string des = js_des;
		int id = js_id;
		int socketid = js_socketid;

		char ftime[30];
		share::formatTime(time(0), ftime);


		LOG_MSG("%s 20:%d  %s id:%d/%d\n", ftime, err, des.c_str(), id, socketid);

		auto c = __TcpServer->client(id);
		if (c == nullptr || c->socketfd != socketid || c->state < func::S_CONNECT_SECURE)
		{
			LOG_MSG("json_reg 没有找到玩家数据...%d %d\n", id, socketid);
			return;
		}
		sendErrInfo(c->ID, CMD_20, err);
	}
}

