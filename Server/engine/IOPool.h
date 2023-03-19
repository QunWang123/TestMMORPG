#ifndef  ____TCPPOOL_H
#define  ____TCPPOOL_H
//������ճ� 
#ifdef ___WIN32_

#include "IDefine.h"

#define ACCEPT_BUF_LENGTH   ((sizeof(struct sockaddr_in) + 16))*2
// ����Զ�˵�ַ�ͱ�����ַ�ĳ���
// +16��ϵͳ�����

class IOContext
{
public:
	IOContext();
	~IOContext();
public:
	WSAOVERLAPPED m_OverLapped;	// �ص��ṹ �൱��һ��ID��Ͷ��ʱ���õ�(IO�������ʱ���ص��ṹ���и�Ӧ)
								// ͨ���и�Ӧ���ص��ṹ�����Ǿ����ҵ���Ӧ��IOContext����
								// ͨ��m_Mode�����Ǿ���׼ȷcast AcceptContext, RecvContext, SendContext
	SOCKET m_Socket;
	int    m_Mode;//���ڼ�¼��ʲôģʽ accept recv send
};

//1��Accept��
class AcceptContext :public IOContext
{
public:
	AcceptContext(int mode, SOCKET listensocket, SOCKET clientsocket);
	~AcceptContext(void);
public:
	SOCKET  listenfd;
	// �µ����ӵ�������Ϣ�ͻᱻ����m_buf����
	unsigned char m_buf[ACCEPT_BUF_LENGTH];
public:
	void clear();
	void setSocket(SOCKET server, SOCKET client);
	static AcceptContext* pop();
	static void push(AcceptContext* acc);
	static int getCount();

};

//2��Recv��
class RecvContext :public IOContext
{
private:
	char* m_Buffs;// �����WSABUF�������ַ��Ļ�����
public:
	WSABUF  m_wsaBuf; //ÿ�εĲ���������

public:
	RecvContext(const int mode);
	~RecvContext(void);
public:
	void clear();
	static int getCount();
	static RecvContext* pop();
	static void push(RecvContext* buff);

};

//3��Send��
class SendContext :public IOContext
{
private:
	char* m_Buffs;// �����WSABUF�������ַ��Ļ�����
public:
	WSABUF  m_wsaBuf; //ÿ�εĲ���������

public:
	SendContext(const int mode);
	~SendContext(void);

public:
	void clear();
	static int getCount();
	int setSend(SOCKET s, char* data, const int sendByte);
	static SendContext* pop();
	static void push(SendContext* buff);
};



#endif
#endif