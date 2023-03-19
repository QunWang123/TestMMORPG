#include "UDPClient.h"



UDPClient_Thread::UDPClient_Thread()
{
	thread = FRunnableThread::Create(this, TEXT("flower thread..."), 0);
}
UDPClient_Thread::~UDPClient_Thread()
{
	if (thread != nullptr)
	{
		delete thread;
		thread = NULL;
	}
}
uint32 UDPClient_Thread::Run()
{
	FString ss = FString::Printf(TEXT("client run thread..."));
	if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);


	__udp->run();
	return 0;
}
void UDPClient_Thread::Exit()
{
	FString ss = FString::Printf(TEXT("client exit thread..."));
	if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 20.0f, FColor::Emerald, ss);
}
void UDPClient_Thread::StopThread()
{
	if (thread != nullptr) thread->WaitForCompletion();

}

//*************************************************************************


void UDPClient::runThread()
{
	isRuning = true;
	m_workthread = new UDPClient_Thread();
}
//Ïß³Ì
void UDPClient::run()
{
	while (isRuning)
	{
		int recvBytes = recvData();
		if (recvBytes >= 0) continue;

		FPlatformProcess::Sleep(0.002);
	}
}
