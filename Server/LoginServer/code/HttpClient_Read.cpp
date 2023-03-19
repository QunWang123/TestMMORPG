#include "HttpClient.h"
//#include "TestProtobuf.h"
//#include "TestBinary.h"
#include "TestJson.h"
//#include "TestLoad.h"
#include <algorithm>
namespace http
{
	int HttpClient::recvSocket()
	{
		memset(m_Response->tempBuf, 0, MAX_ONES_BUF);
		
		int recvBytes = recv(socketfd, m_Response->tempBuf, MAX_ONES_BUF, 0);
		if (recvBytes > 0)
		{
			if (m_Response->pos_head == m_Response->pos_tail)
			{
				m_Response->pos_tail = 0;
				m_Response->pos_head = 0;
			}
			if (m_Response->pos_tail + recvBytes >= MAX_BUF) return -1;
			//保存数据
			memcpy(&m_Response->buf[m_Response->pos_tail], m_Response->tempBuf, recvBytes);
			m_Response->pos_tail += recvBytes;
			return recvBytes;
		}
		//接受数据出错
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
	}
	int HttpClient::analyData()
	{
		if (m_Response->state >= ER_OVER) return 0;
		if (m_Response->state == ER_HEAD)
		{
			//2、解析过数据
			if (m_Response->Content_length <= 0 || m_Response->Content_length > MAX_POST_LENGTH)
			{
				m_Response->state == ER_ERROR;
				return -1;
			}
			readBody();
			return 0;
		}
		if (m_Response->state != ER_FREE) return 0;

		int length = m_Response->pos_tail - m_Response->pos_head;
		m_Response->temp_str.assign(&m_Response->buf[m_Response->pos_head], length);

		int pos = m_Response->temp_str.find("\r\n\r\n");
		if (pos < 0)
		{
			return 0;
		}
		//响应行 响应头 空行
		length = pos + 4;
		m_Response->temp_str.assign(&m_Response->buf[m_Response->pos_head], length);
		std::vector<std::string> arr = split(m_Response->temp_str, "\r\n", false);
		//1、响应行数据
		std::vector<std::string> line = split(arr[0], " ", true);
		if (line.size() == 3)
		{
			auto s = deleteString(line[1], ' ');
			int status = atoi(s.c_str());
			m_Response->SetResponseLine(status, line[2]);
		}
		//2、解析头
		for (int i = 1; i < arr.size() - 1; i++)
		{
			std::vector<std::string>  head = split2(arr[i], ":");
			if (head.size() == 2)
			{
				//转为小写
				std::string key = "";
				std::transform(head[0].begin(), head[0].end(), std::back_inserter(key), ::tolower);

				m_Response->SetHeader(key, head[1]);

				if(strcmp(key.c_str(), "content-length") == 0)
				{
					m_Response->SetContentLength(head[1]);
				}
				if (strcmp(key.c_str(), "content-type") == 0)
				{
					std::vector<std::string>  aaa = split2(head[1], ";");
					if (aaa.size() > 0) m_Response->SetContentType(aaa[0]);
				}
			}
		}
		//3、设置偏移位置
		m_Response->pos_head += (pos + 4);
		m_Response->state == ER_HEAD;
		//输出**************************************************************************************
//#ifdef DEBUG_HTTP
//		LOG_MSG("Reponse===================================%d\n", id);
//		LOG_MSG("%s %d %s\n", m_Response->version.c_str(), m_Response->status, m_Response->describe.c_str());
//		auto it = m_Response->head.begin();
//		while (it != m_Response->head.end())
//		{
//			auto a = it->first;
//			auto b = it->second;
//
//			LOG_MSG("%s:%s\n", a.c_str(), b.c_str());
//			++it;
//		}
//		LOG_MSG("\r\n");
//		//LOG_MSG("%d-%s\n", m_Response->Content_length, m_Response->Content_Type.c_str());
//#endif
		//输出**************************************************************************************

		readBody();
		return 0;
	}
	int HttpClient::readBody()
	{
		int length = m_Response->pos_tail - m_Response->pos_head;
		if (length < m_Response->Content_length) return 0;//需要继续等待粘包


		m_Response->temp_str.clear();
		m_Response->temp_str.assign(&m_Response->buf[m_Response->pos_head], m_Response->Content_length);
		m_Response->pos_head += m_Response->Content_length;
		m_Response->state = ER_FREE;




		//1、解析protobuf
		if (strcmp(m_Response->Content_Type.c_str(), "application/protobuf") == 0)
		{
			//pro::onCommand(this);
			return 0;
		}
		//2、解析2进制自定义数据结构
		if (strcmp(m_Response->Content_Type.c_str(), "application/binary") == 0)
		{
			//binary::onCommand(this);
			return 0;
		}
		//3、解析json
		if (strcmp(m_Response->Content_Type.c_str(), "application/json") == 0)
		{
			//推送到主线程去。。。
			json::pushMainData(m_Response->temp_str);
			return 0;
		}
		//4、下载
		if (strcmp(m_Response->Content_Type.c_str(), "application/load") == 0)
		{
			//load::onCommand(this);
			return 0;
		}

		//m_Response->temp_str.clear();
		//m_Response->temp_str.assign(&m_Response->buf[m_Response->pos_head], m_Response->Content_length);
		
		LOG_MSG("%d-%d %d-%d %s %s\n",
			m_Response->pos_head, m_Response->pos_tail, length, 
			m_Response->Content_length, m_Response->temp_str.c_str(), m_Response->Content_Type.c_str());

		return 0;
	}
}