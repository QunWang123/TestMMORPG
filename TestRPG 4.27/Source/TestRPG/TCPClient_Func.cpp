#include "TcpClient.h"
#include "MyGameInstance.h"
#include "Misc/SecureHash.h"

namespace net
{
	//����
	void TCPClient::parseCommand()
	{
		if (socketfd < 0) return;
		auto c = getData();
		if (c->state < func::C_CONNECT) return;

		//����������
		onHeart();

		while (c->recv_Tail - c->recv_Head > 7)
		{
			//1������ͷ
			uint8 head[2];
			head[0] = c->recvBuf[c->recv_Head] ^ c->rCode;
			head[1] = c->recvBuf[c->recv_Head + 1] ^ c->rCode;

			if (head[0] != func::__ClientInfo->Head[0] || head[1] != func::__ClientInfo->Head[1])
			{
				disconnectServer(2001, "head error...");
				return;
			}

			int32 cl = (*(int32*)(c->recvBuf + c->recv_Head + 2)) ^ c->rCode;
			uint16 cmd = (*(uint16*)(c->recvBuf + c->recv_Head + 6)) ^ c->rCode;

			//2�����Ȳ��� ��Ҫ�����ȴ�
			if (c->recv_Tail < c->recv_Head + cl) break;
			c->recv_TempHead = c->recv_Head + 8;
			c->recv_TempTail = c->recv_Head + cl;
			parseCommand(cmd);
			if (c->state < func::C_CONNECT) return;

			//4�����Ӷ�ȡ����
			c->recv_Head += cl;

			//printf("readdata : %d ..%d:%d.\n", c->State, c->Rece_B, c->Rece_E);
		}
		//��������
		this->onSend();
	}
	void TCPClient::onHeart()
	{
		if (__AppGameInstance == nullptr) return;
		auto c = getData();
		if (c->state < func::C_CONNECT_SECURE) return;

		if (m_data.time_HeartTime > __AppGameInstance->GetTimeSeconds() || m_data.time_HeartTime == 0)
		{
			// �����õģ������ؿ���ʱ��__AppGameInstance->GetTimeSeconds()�ᱻ��Ϊ0����time_HeartTimeȴû��
			m_data.time_HeartTime = __AppGameInstance->GetTimeSeconds();
		}

		
		int32 tempTime = __AppGameInstance->GetTimeSeconds() - m_data.time_HeartTime;
		if (tempTime >= func::__ClientInfo->HeartTime * 1000)
		{
			this->disconnectServer(7000, "lossheart...");
			return;
		}
		
		// Ŀǰ��û��ʱ����������������ݣ��յ�����֮��m_data.time_HeartTime�Ż��޸�
		// 3���Ӹ�����˷�һ����Ϣ
		tempTime = __AppGameInstance->GetTimeSeconds() - m_data.time_Heart;
		if (tempTime >= 3000 ||tempTime < 0)
		{
			m_data.time_Heart = __AppGameInstance->GetTimeSeconds();
			begin(CMD_HEART);
			sss(__AppGameInstance->GetTimeSeconds());
			end();
		}

	}
	void TCPClient::parseCommand(uint16 cmd)
	{
		// �����յ�ʲô��Ϣ�������Ҹ���һ��������
		m_data.time_HeartTime = __AppGameInstance->GetTimeSeconds();

		if (cmd < 65000)
		{
			if (cmd == CMD_HEART)
			{
				int ftime = 0;
				this->read(ftime);
				delaytime = __AppGameInstance->GetTimeSeconds() - ftime;
			}


			if (onCommand != nullptr) onCommand(this, cmd);
			return;
		}

		switch (cmd)
		{
		case CMD_RCODE:
		{
			auto c = getData();
			read(c->rCode);

			FString value = func::__ClientInfo->SafeCode + FString::Printf(TEXT("_%d"), c->rCode);
			c->md5 = FMD5::HashAnsiString(value.GetCharArray().GetData());
			// c->md5 = value;
			TCHAR* pdata = c->md5.GetCharArray().GetData();
			uint8* md5 = (uint8*)TCHAR_TO_UTF8(pdata);

			//����MD5��֤
			begin(CMD_SECURITY);
			sss(0);
			sss(int8(0));
			sss(func::__ClientInfo->Version);
			sss(md5, MAX_MD5_LEN);
			end();
		}
		break;
		case CMD_SECURITY:
		{
			auto c = getData();
			uint16 kind = 0;
			read(kind);
			//printf("-----------client securrity...%d \n", kind);
			if (kind > 0)
			{
				//1 �汾���� 2 MD5���� 
				if (onExceptEvent != nullptr) onExceptEvent(this, kind);
				break;
			}

			c->state = func::C_CONNECT_SECURE;
			if (onSecureEvent != nullptr) onSecureEvent(this, 0);
		}

		break;
		}
	}
	//**********************************************************************
	//**********************************************************************
	void TCPClient::begin(const uint16 cmd)
	{
		auto c = getData();
		//ͷβ���
		if (c->send_Head == c->send_Tail)
		{
			c->send_Tail = 0;
			c->send_Head = 0;
		}
		c->send_TempTail = c->send_Tail;

		if (c->state >= func::C_CONNECT &&
			c->is_Sending == false &&
			socketfd > 0 &&
			c->send_Tail + 8 <= func::__ClientInfo->SendMax)
		{
			c->is_Sending = true;
			c->sendBuf[c->send_Tail + 0] = func::__ClientInfo->Head[0] ^ c->rCode;
			c->sendBuf[c->send_Tail + 1] = func::__ClientInfo->Head[1] ^ c->rCode;

			uint16 newcmd = cmd ^ c->rCode;
			char* a = (char*)& newcmd;
			c->sendBuf[c->send_Tail + 6] = a[0];
			c->sendBuf[c->send_Tail + 7] = a[1];

			c->send_TempTail += 8;
			return;
		}

		disconnectServer(6001, "b error...");
	}

	void TCPClient::end()
	{
		auto c = getData();
		if (c->state == func::C_FREE ||
			c->is_Sending == false ||
			socketfd < 0 ||
			c->send_Tail + 8 > func::__ClientInfo->SendMax ||
			c->send_TempTail > func::__ClientInfo->SendMax ||
			c->send_Tail >= c->send_TempTail)
		{
			disconnectServer(6002, "e error...");
			return;
		}

		c->is_Sending = false;
		uint32 len = (c->send_TempTail - c->send_Tail) ^ c->rCode;
		uint8* a = (uint8*)& len;
		c->sendBuf[c->send_Tail + 2] = a[0];
		c->sendBuf[c->send_Tail + 3] = a[1];
		c->sendBuf[c->send_Tail + 4] = a[2];
		c->sendBuf[c->send_Tail + 5] = a[3];

		//��������ֵ
		c->send_Tail = c->send_TempTail;
	}

	void TCPClient::sss(const int8 v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 1 < func::__ClientInfo->SendMax)
		{
			c->sendBuf[c->send_TempTail] = v;
			c->send_TempTail++;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(const uint8 v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 1 < func::__ClientInfo->SendMax)
		{
			c->sendBuf[c->send_TempTail] = v;
			c->send_TempTail++;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(const int16 v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 2 < func::__ClientInfo->SendMax)
		{
			uint8* p = (uint8*)& v;
			for (int i = 0; i < 2; i++)
				c->sendBuf[c->send_TempTail + i] = p[i];

			c->send_TempTail += 2;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(const uint16 v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 2 < func::__ClientInfo->SendMax)
		{
			uint8* p = (uint8*)& v;
			for (int i = 0; i < 2; i++)
				c->sendBuf[c->send_TempTail + i] = p[i];

			c->send_TempTail += 2;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(const int32 v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 4 < func::__ClientInfo->SendMax)
		{
			uint8* p = (uint8*)& v;
			for (int i = 0; i < 4; i++)
				c->sendBuf[c->send_TempTail + i] = p[i];

			c->send_TempTail += 4;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(const uint32 v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 4 < func::__ClientInfo->SendMax)
		{
			uint8* p = (uint8*)& v;
			for (int i = 0; i < 4; i++)
				c->sendBuf[c->send_TempTail + i] = p[i];

			c->send_TempTail += 4;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(const int64 v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 8 < func::__ClientInfo->SendMax)
		{
			uint8* p = (uint8*)& v;
			for (int i = 0; i < 8; i++)
				c->sendBuf[c->send_TempTail + i] = p[i];

			c->send_TempTail += 8;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(const bool v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 1 < func::__ClientInfo->SendMax)
		{
			uint8* p = (uint8*)& v;
			c->sendBuf[c->send_TempTail] = v;
			c->send_TempTail += 1;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(const float v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 4 < func::__ClientInfo->SendMax)
		{
			uint8* p = (uint8*)& v;
			for (int i = 0; i < 4; i++)
				c->sendBuf[c->send_TempTail + i] = p[i];

			c->send_TempTail += 4;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(const double v)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + 8 < func::__ClientInfo->SendMax)
		{
			uint8* p = (uint8*)& v;
			for (int i = 0; i < 8; i++)
				c->sendBuf[c->send_TempTail + i] = p[i];

			c->send_TempTail += 8;
			return;
		}

		c->is_Sending = false;
	}

	void TCPClient::sss(void* v, const int32 len)
	{
		auto c = getData();
		if (c->is_Sending && c->send_TempTail + len < func::__ClientInfo->SendMax)
		{
			memcpy(&c->sendBuf[c->send_TempTail], v, len);

			c->send_TempTail += len;
			return;
		}
		c->is_Sending = false;
	}
	//*******************************************************************
	//*******************************************************************
	//*******************************************************************
	bool isValid(S_SERVER_BASE* c, int32 value)
	{
		if (c->state == func::C_FREE ||
			c->recv_TempTail == 0 ||
			c->recv_TempHead + value > c->recv_TempTail)
		{
			return false;
		}
		return true;
	}

	void TCPClient::read(int8& v)
	{
		auto c = getData();
		if (isValid(c, 1) == false)
		{
			v = 0;
			return;
		}
		v = (*(int8*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead++;
	}

	void TCPClient::read(uint8 & v)
	{
		auto c = getData();
		if (isValid(c, 1) == false)
		{
			v = 0;
			return;
		}
		v = (*(uint8*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead++;
	}

	void TCPClient::read(int16 & v)
	{
		auto c = getData();
		if (isValid(c, 2) == false)
		{
			v = 0;
			return;
		}
		v = (*(int16*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead += 2;
	}

	void TCPClient::read(uint16 & v)
	{
		auto c = getData();
		if (isValid(c, 2) == false)
		{
			v = 0;
			return;
		}
		v = (*(uint16*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead += 2;
	}

	void TCPClient::read(int32 & v)
	{
		auto c = getData();
		if (isValid(c, 4) == false)
		{
			v = 0;
			return;
		}
		v = (*(int32*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead += 4;
	}

	void TCPClient::read(uint32 & v)
	{
		auto c = getData();
		if (isValid(c, 4) == false)
		{
			v = 0;
			return;
		}
		v = (*(uint32*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead += 4;
	}

	void TCPClient::read(int64 & v)
	{
		auto c = getData();
		if (isValid(c, 8) == false)
		{
			v = 0;
			return;
		}
		v = (*(int64*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead += 8;
	}


	void TCPClient::read(bool& v)
	{
		auto c = getData();
		if (isValid(c, 1) == false)
		{
			v = 0;
			return;
		}
		v = (*(bool*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead += 1;
	}

	void TCPClient::read(float & v)
	{
		auto c = getData();
		if (isValid(c, 4) == false)
		{
			v = 0;
			return;
		}
		v = (*(float*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead += 4;
	}

	void TCPClient::read(double & v)
	{
		auto c = getData();
		if (isValid(c, 8) == false)
		{
			v = 0;
			return;
		}
		v = (*(double*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead += 8;
	}

	void TCPClient::read(void* v, const int32 len)
	{
		auto c = getData();
		if (isValid(c, len) == false)
		{
			v = 0;
			return;
		}
		memcpy(v, &c->recvBuf[c->recv_TempHead], len);
		c->recv_TempHead += len;
	}


	void TCPClient::setOnConnect(TCPCLIENTNOTIFY_EVENT event)
	{
		onAcceptEvent = event;
	}

	void TCPClient::setOnSecureConnect(TCPCLIENTNOTIFY_EVENT event)
	{
		onSecureEvent = event;
	}

	void TCPClient::setOnDisConnect(TCPCLIENTNOTIFY_EVENT event)
	{
		onDisconnectEvent = event;
	}

	void TCPClient::setOnExcept(TCPCLIENTNOTIFY_EVENT event)
	{
		onExceptEvent = event;
	}

	void TCPClient::setOnCommand(TCPCLIENTNOTIFY_EVENT event)
	{
		onCommand = event;
	}
}
