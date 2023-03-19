#include "HttpServer.h"
#include <string>
#include "TestJson.h"

//#include "TestProtobuf.h"
//#include "TestBinary.h"
//#include "TestJson.h"
//#include "TestPut.h"
#include <algorithm>
#include "ShareFunction.h"

using namespace std::chrono;


namespace http
{
	//接收数据 粘包处理
	int HttpServer::recvSocket(Socket socketfd, S_HTTP_BASE* quest)
	{
		memset(quest->tempBuf, 0, MAX_ONES_BUF);

		int recvBytes = recv(socketfd, quest->tempBuf, MAX_ONES_BUF, 0);
		if (recvBytes > 0)
		{
			if (quest->pos_head == quest->pos_tail)
			{
				quest->pos_tail = 0;
				quest->pos_head = 0;
			}
			if (quest->pos_tail + recvBytes >= MAX_BUF) return -1;
			memcpy(&quest->buf[quest->pos_tail], quest->tempBuf, recvBytes);
			quest->pos_tail += recvBytes;
			return 0;
		}
#ifdef ___WIN32_
		if (recvBytes < 0)
		{
			int err = WSAGetLastError();
			if (err == WSAEINTR) return 0;
			else if (err == WSAEWOULDBLOCK) return 0;
			else return -1;
		}
		else if (recvBytes == 0)
		{
			return -2;
		}
#else
		if (recvBytes < 0)  //出错
		{
			if (errno == EINTR) return 0; //被信号中断
			else if (errno == EAGAIN)return 0;//没有数据 请稍后再试
			else return -1;
		}
		else if (recvBytes == 0)
		{
			return -2;
		}
#endif
	}
	int HttpServer::analyData(Socket socketfd, S_HTTP_BASE* quest, S_HTTP_BASE* reponse)
	{
		if (quest->state >= ER_OVER) return 0;

		if (quest->state == ER_HEAD)
		{
			if ((quest->method != "POST" && quest->method != "PUT") && (quest->Content_length <= 0 || quest->Content_length > MAX_POST_LENGTH))
			{
				quest->state == ER_ERROR;
				//返回错误
				reponse->SetResponseLine(403, "Failed");
				this->writeData(quest, reponse, "err", 3);
				return -1;
			}
			//读取消息体
			readBody(socketfd, quest, reponse);
			return 0;
		}
		if (quest->state != ER_FREE) return 0;

		//1、没有解析过数据
		int length = quest->pos_tail - quest->pos_head;

		quest->temp_str.clear();
		quest->temp_str.assign(&quest->buf[quest->pos_head], length);

		//2、解析请求行 请求头 请求空行
		int pos = quest->temp_str.find("\r\n\r\n");
		if (pos < 0)
		{
			if (quest->method != "PUT")
			if (length >= MAX_PACKAGE_LENGTH)
			{
				quest->state == ER_ERROR;

				//返回错误
				reponse->SetResponseLine(401, "Failed");
				this->writeData(quest, reponse, "err", 3);
				return -1;
			}

			return 0;
		}
		length = pos + 4;
		quest->temp_str.clear();
		quest->temp_str.assign(&quest->buf[quest->pos_head], length);
		std::vector<std::string> arr = split(quest->temp_str, "\r\n", false);

		//1、请求行数据
		std::vector<std::string> line = split(arr[0], " ", true);
		quest->SetRequestLine(line);
		//2、解析头
		for (int i = 1; i < arr.size() - 1; i++)
		{
			std::vector<std::string>  head = split2(arr[i], ":");
			if (head.size() == 2)
			{
				//转为小写
				std::string key = "";
				std::transform(head[0].begin(), head[0].end(), std::back_inserter(key), ::tolower);
		
				quest->SetHeader(key, head[1]);

				if (strcmp(key.c_str(), "content-length") == 0)
				{
					quest->SetContentLength(head[1]);
				}
				if (strcmp(key.c_str(), "content-type") == 0)
				{
					std::vector<std::string>  aaa = split2(head[1], ";");
					if (aaa.size() > 0) quest->SetContentType(aaa[0]);
				}
				if (strcmp(key.c_str(), "connection") == 0)
				{
					quest->SetConnection(head[1]);
				}
			}
		}
		//3、设置偏移位置
		quest->pos_head += (pos + 4);

		//输出**************************************************************************************
#ifdef DEBUG_HTTP
		char ftime[30];
		share::formatTime(time(0), ftime);

		LOG_MSG("Quest [%s]==========================%d-%d\n", ftime,(int)socketfd, quest->threadid);
		LOG_MSG("%s %s %s\n", quest->method.c_str(), quest->url.c_str(), quest->version.c_str());

		auto it = quest->head.begin();
		while (it != quest->head.end())
		{
			auto a = it->first;
			auto b = it->second;
			LOG_MSG("%s:%s\n", a.c_str(), b.c_str());
			++it;
		}
		LOG_MSG("\r\n");
		//LOG_MSG("%d-%s-%s\n", quest->Content_length, quest->Content_Type.c_str(), quest->Connection.c_str());
#endif // DEBUG
		//输出**************************************************************************************
		//4、提交的是post put 
		if (quest->method != "POST" && quest->method != "PUT")
		{
			quest->state = ER_OVER;

			quest->temp_str.clear();
			bool isexist = read_Quest(quest->url, quest->temp_str);
			if (isexist)
			{
				quest->loadput = EL_LOAD;
				//quest->SetContentType("application/load");
				//quest->SetHeader("Content-Type", "application/load");
				
				reponse->SetResponseLine(200, quest->url);
				this->writeData(quest, reponse, quest->temp_str.c_str(), quest->temp_str.size());
			}
			else
			{
				//获取数据
				reponse->SetResponseLine(404, "Failed");
				this->writeData(quest, reponse, "err", 3);
			}


			return 0;
		}
		//是上传PUT 方法
		if (strcmp(quest->method.c_str(), "PUT") == 0)
		{
			quest->loadput = EL_PUT;
		}

		quest->state = ER_HEAD;
		if (quest->Content_length <= 0 || quest->Content_length > MAX_POST_LENGTH)
		{
			quest->state = ER_ERROR;
			//返回错误
			reponse->SetResponseLine(402, "Failed");
			this->writeData(quest, reponse, "err", 3);
			return -1;
		}

		//读取消息体
		readBody(socketfd, quest, reponse);
		return 0;
	}
	int HttpServer::readBody(Socket socketfd, S_HTTP_BASE* quest, S_HTTP_BASE* reponse)
	{
		int length = quest->pos_tail - quest->pos_head;
		if (length < quest->Content_length) return 0;//需要继续等待解包

		//0、如果是上传资源
		if (strcmp(quest->method.c_str(), "PUT") == 0)
		{
			//put::onCommand(this, quest, reponse);
			return 0;
		}
		//1、解析protobuf
		if (strcmp(quest->Content_Type.c_str(), "application/protobuf") == 0)
		{
			//pro::onCommand(this, quest, reponse);
			return 0;
		}
		//2、解析2进制数据结构
		if (strcmp(quest->Content_Type.c_str(), "application/binary") == 0)
		{
			//binary::onCommand(this, quest, reponse);
			return 0;
		}
		//3、解析json
		if (strcmp(quest->Content_Type.c_str(), "application/json") == 0)
		{
			json::onCommand(this, quest, reponse);
			return 0;
		}

		std::string body(quest->buf, quest->pos_head, quest->Content_length);
		quest->pos_head += quest->Content_length;
		quest->state = ER_OVER;
#ifdef DEBUG_HTTP
		LOG_MSG("readBody %d-%d %s-%d \n", quest->pos_head, quest->pos_tail, body.c_str(), body.size());
#endif

		//响应数据给前端
		reponse->SetResponseLine(200, "OK");
		this->writeData(quest, reponse, "ok", 2);
		return 0;
	}
}