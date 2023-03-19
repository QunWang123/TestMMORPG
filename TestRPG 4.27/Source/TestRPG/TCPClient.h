#ifndef  ____TCPCLIENT_H
#define  ____TCPCLIENT_H

#include "IDefine.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"


namespace net
{
	class TCPClient;
	typedef void(*TCPCLIENTNOTIFY_EVENT) (TCPClient* tcp, const int32 code);

	class TCPClient_Thread : public FRunnable
	{
	public:
		TCPClient_Thread(TCPClient* c);
		virtual ~TCPClient_Thread();
		
		TCPClient* tcp;

		void StopThread();
		virtual uint32 Run();
		virtual void Exit();

		FRunnableThread* thread;

	};

	class TCPClient
	{
		bool isFirstConnect;
		bool isRunning;
		bool isPause;
		
		FSocket* socketfd;
		S_SERVER_BASE  m_data;
		TCPClient_Thread* m_workthread;

		TCPCLIENTNOTIFY_EVENT      onAcceptEvent;
		TCPCLIENTNOTIFY_EVENT      onSecureEvent;
		TCPCLIENTNOTIFY_EVENT      onDisconnectEvent;
		TCPCLIENTNOTIFY_EVENT      onExceptEvent;
		TCPCLIENTNOTIFY_EVENT      onCommand;

		int32 initSocket();
		int32 onRecv();
		int32 onSend();

		void  onHeart();
		void  parseCommand(uint16 cmd);

	public:
		TCPClient();
		~TCPClient();
		inline S_SERVER_BASE* getData() { return &m_data; };

		void run();
		void stop();
		void setThread(bool ispause);

		void resetIP(FString newip, int32 newport);

		void runClient(int32 sid, FString ip, int32 port);
		bool connectServer();
		void disconnectServer(const int32 errcode, FString err);

		void  begin(const uint16 cmd);
		void  end();
		void  sss(const int8 v);
		void  sss(const uint8 v);
		void  sss(const int16 v);
		void  sss(const uint16 v);
		void  sss(const int32 v);
		void  sss(const uint32 v);
		void  sss(const int64 v);
		void  sss(const bool v);
		void  sss(const float v);
		void  sss(const double v);
		void  sss(void* v, const int32 len);

		void  read(int8& v);
		void  read(uint8& v);
		void  read(int16& v);
		void  read(uint16& v);
		void  read(int32& v);
		void  read(uint32& v);
		void  read(int64& v);
		void  read(bool& v);
		void  read(float& v);
		void  read(double& v);
		void  read(void* v, const int32 len);

		void  parseCommand();
		void  setOnConnect(TCPCLIENTNOTIFY_EVENT event);
		void  setOnSecureConnect(TCPCLIENTNOTIFY_EVENT event);
		void  setOnDisConnect(TCPCLIENTNOTIFY_EVENT event);
		void  setOnExcept(TCPCLIENTNOTIFY_EVENT event);
		void  setOnCommand(TCPCLIENTNOTIFY_EVENT event);
	};
}

#endif