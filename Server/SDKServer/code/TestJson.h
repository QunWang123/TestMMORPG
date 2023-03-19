#pragma once
#include "IDefine.h"
#include "HttpServer.h"
using namespace http;

namespace json
{
	extern void onCommand(HttpServer* server, S_HTTP_BASE* quest, S_HTTP_BASE* reponse);

}