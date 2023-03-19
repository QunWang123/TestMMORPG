#ifndef  ____TCPPOOL_H
#define  ____TCPPOOL_H
//对象回收池 
#ifdef ___WIN32_

#include "IDefine.h"

#define ACCEPT_BUF_LENGTH   ((sizeof(struct sockaddr_in) + 16))*2
// 包含远端地址和本机地址的长度
// +16是系统保存的

class IOContext
{
public:
	IOContext();
	~IOContext();
public:
	WSAOVERLAPPED m_OverLapped;	// 重叠结构 相当于一个ID，投递时候用的(IO操作完成时，重叠结构会有感应)
								// 通过有感应的重叠结构，我们就能找到对应的IOContext对象
								// 通过m_Mode，我们就能准确cast AcceptContext, RecvContext, SendContext
	SOCKET m_Socket;
	int    m_Mode;//用于记录是什么模式 accept recv send
};

//1、Accept类
class AcceptContext :public IOContext
{
public:
	AcceptContext(int mode, SOCKET listensocket, SOCKET clientsocket);
	~AcceptContext(void);
public:
	SOCKET  listenfd;
	// 新得连接的数据信息就会被存在m_buf里面
	unsigned char m_buf[ACCEPT_BUF_LENGTH];
public:
	void clear();
	void setSocket(SOCKET server, SOCKET client);
	static AcceptContext* pop();
	static void push(AcceptContext* acc);
	static int getCount();

};

//2、Recv类
class RecvContext :public IOContext
{
private:
	char* m_Buffs;// 这个是WSABUF里具体存字符的缓冲区
public:
	WSABUF  m_wsaBuf; //每次的操作缓冲区

public:
	RecvContext(const int mode);
	~RecvContext(void);
public:
	void clear();
	static int getCount();
	static RecvContext* pop();
	static void push(RecvContext* buff);

};

//3、Send类
class SendContext :public IOContext
{
private:
	char* m_Buffs;// 这个是WSABUF里具体存字符的缓冲区
public:
	WSABUF  m_wsaBuf; //每次的操作缓冲区

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