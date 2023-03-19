#ifndef  ____UDPCLIENT_H
#define  ____UDPCLIENT_H

#include "INetBase_Udp.h"

using namespace func;

namespace net
{
	class UdpClient:public IUdpClient
	{
	public:
		UdpClient();
		virtual ~UdpClient();
		virtual int   runClient(char* ip,u16 port,u32 id);
		virtual void  stopClient();
		virtual int   sendData(const char* buf, const u32 size, const char* ip, const int port);
		virtual int   sendData(const void* buf, const u32 size, const u8 protocolType);
		virtual void  clearConnect();
		virtual void  readData(void* v, const u32 len);
		virtual S_UDP_BASE* getData();
		virtual void  parseCommand();
		virtual void  connectServer();
		virtual void  registerCommand(int cmd, void* container);
		virtual void  setOnUdpServerAccept(UDPCLIENTNOTIFY_EVENT event);
		virtual void  setOnUdpServerDisconnect(UDPCLIENTNOTIFY_EVENT event);


	private:
		void  initCommands();
		int   initSocket();
		void  runThread();
		int   recvData();
		
		void  checkConnect();
		void  sendHeart();
		void  parseCommand(u16 cmd);
		void  setAddress(char* ip, int port);
		void  onAccept(S_DATA_HEAD& head, sockaddr_in& addr);
		void  onDisconnect(S_DATA_HEAD& head, sockaddr_in& addr);
		void  onRecv_SaveData(char* buf, S_UDP_BASE* c, const u32 recvBytes);
		
		static void run(UdpClient* udp);

		int   recvData_kcp(char* buf, s32 recvBytes);
		void createKcp(s32 id);
		void updateKcp();
		void releaseKcp();
	private:
		UDPSOCKET socketfd;
		std::shared_ptr<std::thread> m_workthread;
		S_UDP_BASE  data;

		UDPCLIENTNOTIFY_EVENT      onAcceptEvent;
		UDPCLIENTNOTIFY_EVENT      onDisconnectEvent;

	};

	
}

#endif