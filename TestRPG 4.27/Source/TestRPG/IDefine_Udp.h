#ifndef __IDEFINE_UDP_H
#define __IDEFINE_UDP_H

#include "IDefine.h"
#include "CoreMinimal.h"
#include "ikcp.h"
#include "sockets.h"

#define MAX_UDP_BUF   512
#define MAX_IP_LEN   20 
#define CMD_65001 65001
#define CMD_65002 65002
#define CMD_65003 65003

namespace func
{
	//struct ConfigXML
	//{
	//	int32   ID;     //������ID
	//	int8    Type;   //���������� 1-DB 2-���ķ����� 3-��ͼ������ 4-���ط����� 
	//	uint16   Port;   //�������˿ں�
	//	int32   MaxUser;//����������
	//	int32   MaxConnect;//���ͻ�����������
	//	uint8    RCode;
	//	int32   Version;
	//	int32   ReceOne;
	//	int32   ReceMax;
	//	int32   SendOne;
	//	int32   SendMax;
	//	int32   HeartTime;//����ʱ��
	//	int32   HeartTimeMax;//����ʱ��
	//	int32   AutoTime;//�Զ�����ʱ��
	//	int32   MaxAccpet; //���Ͷ����������
	//	int32   MaxRece;   //����յ���Ϣ����
	//	int32   MaxSend;   //�������Ϣ����
	//	char  SafeCode[20];
	//	char  Head[3];
	//	

	//};

	extern ConfigXML* __UDPClientInfo;
}
namespace net
{
	enum S_UDP_STATE
	{
		S_UFree = 0,
		S_UConnect = 1,
		S_ULogin = 2
	};
	enum S_PROTOCOL_TYPE
	{
		SPT_UDP = 0,
		SPT_KCP = 1
	};
#pragma pack(push,packing)
#pragma pack(1)


	struct S_UDP_BASE
	{
		int8      state;//0δ���� 1�ѽ������� 
		int32     ID;//����ID
		uint32     ip;
		uint16		port;
		uint32     threadID;
		FString    strip;

		bool	is_RecvCompleted;
		uint8*  recvBuf;
		int32		recv_Head;//ͷ ������ʹ��
		int32		recv_Tail;//β ������ʹ��
		int32     recv_TempHead;
		int32     recv_TempTail;
		int32     time_Heart;//������ ʱ��
		int32     time_AutoConnect;//�Զ�����
		int32     time_HeartConnect;
		//sockaddr_in addr; //�ͻ��˵�ַ
		ikcpcb* kcp;
		FSocket* UdpSocket;

		void init(int32 length);
		void reset();

		inline bool isT(uint32 id)
		{
			if (id == ID) return true;
			return false;
		}
		inline bool isT(uint32 ipvalue, int32 portvalue)
		{
			if (ip == ipvalue && port == portvalue) return true;
			return false;
		}
	};

	struct S_DATA_HEAD
	{
		int32   ID;
		uint16   cmd;
		inline void setSecure(uint8 code)
		{
			ID = ID ^ code;
			cmd = cmd ^ code;
		}
		inline void getSecure(uint8 code)
		{
			ID = ID ^ code;
			cmd = cmd ^ code;
		}
	};
	struct S_DATA_BASE
	{
		char head[2];
		uint16  length;
		uint8 buf[MAX_UDP_BUF];
		inline void reset()
		{
			memset(this, 0, sizeof(S_DATA_BASE));
		}
	};

	struct S_DATA_TEST :public S_DATA_HEAD
	{
		int32    a;
		uint16   b;
		uint8    s[20];
		uint8    state;
	};
#pragma pack(pop, packing)



}

#endif