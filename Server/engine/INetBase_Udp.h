#ifndef  ____INETBASE_UDP_H
#define  ____INETBASE_UDP_H 

#include "IDefine_Udp.h"
#include "ikcp.h"

namespace net
{
	//服务器
	enum S_USOCKET_STATE
	{
		S_UFree = 0,
		S_UConnect = 1, //安全连接
		S_ULogin = 2// 登录
	};
	enum S_PROTOCOL_TYPE
	{
		SPT_UDP = 0,
		SPT_KCP = 1
	};
#pragma pack(push,packing)
#pragma pack(1)
//code

	struct S_UDP_BASE
	{
		u8      state;//0未连接 1已建立连接 
		s32     ID;//连接ID
		UDPSOCKET  socketfd;
		char    strip[16];
		u32     ip;
		u16		port;
		u32     threadID;
		u64     memid;

		bool	is_RecvCompleted;
		char*   recvBuf;
		s32		recv_Head;//头 消费者使用
		s32		recv_Tail;//尾 生产者使用
		s32     recv_TempHead;
		s32     recv_TempTail;
		s32     time_Heart;//心跳包 时间
		s32     time_HeartConnect;//心跳包 时间
		s32     time_AutoConnect;//自动连接
		sockaddr_in addr; //客户端地址
		ikcpcb* kcp;

		void init(u32 length);
		void reset();
		void initdata();

		inline bool isT(s32 id)
		{
			if (id == ID) return true;
			return false;
		}
		inline bool isT(u32 ipvalue,u16 portvalue)
		{
			if (ip == ipvalue && port == portvalue) return true;
			return false;
		}
	};




#pragma pack(pop, packing)

	class IUdpServer;
	class IUdpClient;
	typedef void(*UDPSERVERNOTIFY_EVENT)(IUdpServer* udp, S_UDP_BASE* c, const s32 code, const char* err);
	typedef void(*UDPCLIENTNOTIFY_EVENT)(IUdpClient* udp, const s32 code, const char* err);
	

	//定义我们的服务器接口 纯虚函数 
	class IUdpServer
	{
	public:
		virtual ~IUdpServer() {}
		virtual int   runServer() = 0;
		virtual void  stopServer() = 0;
		virtual int   sendData(const void* buf, const u32 size, const char* ip, const int port) = 0;
		virtual int   sendData(const s32 id, const void* buf, const u32 size, const u8 protocolType) = 0;
		virtual void  readData(const int id, void* v, const u32 len) = 0;
		virtual int   getConnectCount() = 0;
		virtual void  clearConnect(S_UDP_BASE* c) = 0;
		virtual void  parseCommand() = 0;
		virtual void  registerCommand(int cmd, void* container) = 0;
		virtual void  setOnUdpClientAccept(UDPSERVERNOTIFY_EVENT event) = 0;
		virtual void  setOnUdpClientDisconnect(UDPSERVERNOTIFY_EVENT event) = 0;

		virtual S_UDP_BASE* findClient(const int id) = 0;
		virtual S_UDP_BASE* findClient(const int id, int state) = 0;
		virtual S_UDP_BASE* findClient(const int id, u32 ip, u16 port) = 0;
	};


	class IUdpClient
	{
	public:
		virtual ~IUdpClient() {}
		virtual int  runClient(char* ip, u16 port,u32 id) = 0;
		virtual void  stopClient() = 0;
		virtual int   sendData(const char* buf,const u32 size,const char* ip,const int port) = 0;
		virtual int   sendData(const void* buf,const u32 size, const u8 protocolType) = 0;
		virtual void  readData(void* v, const u32 len) = 0;
		virtual void  clearConnect() = 0;
		virtual S_UDP_BASE* getData() = 0;
		virtual void  parseCommand() = 0;
		virtual void  connectServer() = 0;
		virtual void  registerCommand(int cmd, void* container) = 0;
		virtual void  setOnUdpServerAccept(UDPCLIENTNOTIFY_EVENT event) = 0;
		virtual void  setOnUdpServerDisconnect(UDPCLIENTNOTIFY_EVENT event) = 0;
	};


	extern IUdpServer* NewUdpServer();
	extern IUdpClient* NewUdpClient();

}


#endif