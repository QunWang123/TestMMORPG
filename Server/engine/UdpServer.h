#ifndef  ____UDPSERVER_H
#define  ____UDPSERVER_H


#include "INetBase_Udp.h"

#include "IContainer.h"

#include <map>
#include <mutex>

using namespace func;

namespace net
{
	class UdpServer:public IUdpServer
	{
	public:
		UdpServer();
		virtual ~UdpServer();
		virtual int   runServer();
		virtual void  stopServer();
		virtual int   sendData(const void* buf, const u32 size,const char* ip,const int port);
		virtual int   sendData(const s32 id, const void* buf, const u32 size, const u8 protocolType);
		virtual void  readData(const int id, void* v, const u32 len);
		virtual int   getConnectCount();
		virtual void  clearConnect(S_UDP_BASE* c);

		virtual void  parseCommand();
		virtual void  registerCommand(int cmd, void* container);
		virtual void  setOnUdpClientAccept(UDPSERVERNOTIFY_EVENT event);
		virtual void  setOnUdpClientDisconnect(UDPSERVERNOTIFY_EVENT event);
	private:
		void  initCommands();
		int   initSocket();
		void  runThread();
		int   recvData();
		
		void  onAccept(char* buf, S_DATA_HEAD& head, sockaddr_in& clientAddr);
		void  onRecv_SaveData(char* buf, S_UDP_BASE* c,  const u32 recvBytes);
		
		void  checkConnect(S_UDP_BASE* c);
		S_UDP_BASE*  findFree();//分配一个连接ID
		bool  checkIsConnect(u32 ip, u16 port);
		void  addCheck(u32 ip, u16 port,s32 id);
		void  deleteCheck(u32 ip, u16 port);
		bool  checkIsSendTime(u32 ip, u16 port);
		void  addCheckSendTime(u32 ip, u16 port);
		void  sendHeart(S_UDP_BASE* c);
		static void run(UdpServer* udp);

		int   recvData_kcp(char* buf, s32 recvBytes,s32 ip,u16 port);
		void createKcp(S_UDP_BASE* c);
		void updateKcp(S_UDP_BASE* c);
		void releaseKcp(S_UDP_BASE* c);

	private:
		UDPSOCKET socketfd;
		std::shared_ptr<std::thread> m_workthread;
		HashArray<S_UDP_BASE>* Linkers;//连接玩家
		std::map<u64, int>   m_CheckConnect;//检查连接时间容器
		std::map<u64, int>   m_CheckSendTime;//检查心跳时间
		std::mutex           m_CheckConnectMutex;
	

		UDPSERVERNOTIFY_EVENT      onAcceptEvent;
		UDPSERVERNOTIFY_EVENT      onDisconnectEvent;

		void  parseCommand(S_UDP_BASE* c);
		void  parseCommand(S_UDP_BASE* c, const u16 cmd);
	public:
		virtual S_UDP_BASE* findClient(const int id);
		virtual S_UDP_BASE* findClient(const int id, int state);
		virtual S_UDP_BASE* findClient(const int id, u32 ip, u16 port);
	};

}

#endif