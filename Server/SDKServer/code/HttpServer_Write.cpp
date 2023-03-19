#include "HttpServer.h"
#include <string>
using namespace std::chrono;


namespace http
{
	//获取响应行 响应头 空行
	std::string getReponseStr(S_HTTP_BASE* quest, S_HTTP_BASE* reponse)
	{
		std::string stream;
		//1、响应行
		stream += reponse->version + " " + std::to_string(reponse->status) + " " + reponse->describe + "\r\n";
		//2、响应头
		auto it = quest->head.begin();
		while (it != quest->head.end())
		{
			auto key = it->first;
			auto value = it->second;
			stream += key + ":" + value + "\r\n";
			++it;
		}
		//3、响应空行
		stream += "\r\n";
		return stream;
	}
	//封包 写数据
	void HttpServer::writeData(S_HTTP_BASE* quest, S_HTTP_BASE* reponse, const char* body, int size)
	{
		if (reponse->state != ES_FREE) return;
		if (body == NULL) return;
		if (size <= 0 || size > MAX_ONES_BUF) return;

		//1、设置响应数据消息体长度
		//quest->SetHeader("Content-Length", std::to_string(size));
		quest->SetHeader("content-length", std::to_string(size));
		//2、设置响应
		reponse->state = ES_SENDING;

		std::string stream = getReponseStr(quest, reponse);
		int size2 = stream.size();


#ifdef DEBUG_HTTP
		LOG_MSG("Response===================================%d\n", quest->threadid);
		LOG_MSG("%s%s\n", stream.c_str(), body);
#endif
		//填充数据
		if (reponse->pos_tail + size2 + size < MAX_BUF)
		{
			memcpy(&reponse->buf[reponse->pos_tail], stream.c_str(), size2);
			reponse->pos_tail += size2;

			memcpy(&reponse->buf[reponse->pos_tail], body, size);
			reponse->pos_tail += size;
		}
	}
	int HttpServer::sendSocket(Socket socketfd, S_HTTP_BASE* reponse, int tid)
	{
		if (reponse->state != ES_SENDING) return 0;
		int len = reponse->pos_tail - reponse->pos_head;
		if (len <= 0) return 0;
		
		int sendBytes = send(socketfd, &reponse->buf[reponse->pos_head], len, 0);
		if (sendBytes > 0)
		{
			reponse->pos_head += sendBytes;
			if (reponse->pos_head == reponse->pos_tail)
			{
				//初始化响应数据
				reponse->Reset();
				reponse->state = ES_OVER;
			}
			return 0;
		}

		LOG_MSG("sendSocket err %d-%d\n", len, sendBytes);
#ifdef ___WIN32_
		if (sendBytes < 0)
		{
			int err = WSAGetLastError();
			if (err == WSAEINTR) return 0;
			else if (err == WSAEWOULDBLOCK) return 0;
			else return -1;
		}
		else if (sendBytes == 0)
		{
			return -2;
		}
#else
		if (sendBytes < 0)  //出错
		{
			if (errno == EINTR) return 0; //被信号中断
			else if (errno == EAGAIN)return 0;//没有数据 请稍后再试
			else return -1;
		}
		else if (sendBytes == 0)
		{
			return -2;
		}
#endif
	}
}