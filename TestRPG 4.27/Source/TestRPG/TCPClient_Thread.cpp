#include "TcpClient.h"
#include "CoreMinimal.h"
#include "Engine/Engine.h"


namespace net
{
	TCPClient_Thread::TCPClient_Thread(TCPClient* c)
	{
		this->tcp = c;
		thread = FRunnableThread::Create(this, TEXT("TestThread///"), 0);
	}

	TCPClient_Thread::~TCPClient_Thread()
	{
		if (thread != nullptr)
		{
			delete thread;
			thread = NULL;
		}
	}

	uint32 TCPClient_Thread::Run()
	{
		FString ss = FString::Printf(TEXT("client run thread..."));
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.f, FColor::Emerald, ss);
		tcp->run();
		return 0;
	}

	void TCPClient_Thread::Exit()
	{
		FString ss = FString::Printf(TEXT("client exit thread..."));
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.f, FColor::Emerald, ss);
	}

	void TCPClient_Thread::StopThread()
	{
		tcp->stop();
		if (thread != nullptr) thread->WaitForCompletion();
	}

	/******************************************************************/

	void TCPClient::stop()
	{
		isRunning = false;
	}

	void TCPClient::setThread(bool ispause)
	{
		isPause = ispause;
	}

	void TCPClient::run()
	{
		while (isRunning)
		{
			if (!isPause)
			{
				switch (m_data.state)
				{	
				case func::C_FREE:
					{
						if (isFirstConnect == false)
						{
							m_data.reset();
							FPlatformProcess::Sleep(func::__ClientInfo->AutoTime);
							FString ss = FString::Printf(TEXT("auto connect..."));
							if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.f, FColor::Emerald, ss);
						}
						else
						{
							isFirstConnect = false;
						}
						this->connectServer();
					}
					break;
				default:
					this->onRecv();
					break;
				}
			}

			FPlatformProcess::Sleep(0.03);
		}
	}
}	