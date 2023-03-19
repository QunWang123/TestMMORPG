#include "HttpClient.h"
#include <string>
namespace http
{
	//添加数据
	void HttpClient::pushRequest(std::string method, std::string url, int type, const char* c, const int len)
	{
		char content_length[30];
		memset(content_length, 0, 30);
		sprintf(content_length, "Content-Length:%d\r\n", len);

		std::string content_type;
		switch (type)
		{
		case 1:content_type = "Content-Type:application/protobuf; charset=utf-8\r\n"; break;
		case 2:content_type = "Content-Type:application/binary; charset=utf-8\r\n"; break;
		case 3:content_type = "Content-Type:application/json; charset=utf-8\r\n"; break;
		default:content_type = "Content-Type:application/text; charset=utf-8\r\n"; break;
		}

		std::string stream;
		//请求行
		stream += method + " " + url + " HTTP/1.1\r\n"; 
		//请求头
		stream += "Host: " + serverip + ":" + std::to_string(serverport) + "\r\n";
		stream += content_length;
		stream += content_type;
		stream += "Connection:close\r\n";
		//空行
		stream += "\r\n";



		S_TEST_BASE* data = popPool();
		data->Init();

		int index = 0;
		//请求行 请求头 空行
		memcpy(&data->buf[index], stream.c_str(), stream.size());
		index += stream.size();
		data->len = stream.size();

		if (method == "PUT" || method == "POST")
		{
			memcpy(&data->buf[index], c, len);
			data->len = stream.size() + len;
		}

		{
			std::unique_lock<std::mutex> guard(this->m_Mutex);
			this->m_HttpDatas.push_back(data);
		}
	}
	//生产数据
	void HttpClient::writeData(S_TEST_BASE* data)
	{
		if (m_Request->pos_head == m_Request->pos_tail)
		{
			m_Request->pos_tail = 0;
			m_Request->pos_head = 0;
		}

		if (m_Request->pos_tail + data->len < MAX_BUF)
		{
			memcpy(&m_Request->buf[m_Request->pos_tail], data->buf, data->len);
			m_Request->pos_tail += data->len;
		}
	}
	//发包
	int HttpClient::sendSocket()
	{
		int len = m_Request->pos_tail - m_Request->pos_head;
		if (len <= 0) return 0;

		int sendBytes = send(socketfd, &m_Request->buf[m_Request->pos_head], len, 0);
		if (sendBytes > 0)
		{
			m_Request->pos_head += sendBytes;
			return 0;
		}

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
	}
}