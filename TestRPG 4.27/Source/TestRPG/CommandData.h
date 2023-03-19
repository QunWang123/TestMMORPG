#ifndef  __COMMANDDATA_H
#define  __COMMANDDATA_H

#include "Containers/UnrealString.h"

#define USER_MAX_MEMBER			20
#define USER_MAX_PASS			20
#define USER_MAX_KEY			11
#define USER_MAX_ROLE			3
#define USER_MAX_NICK			20

#define  CMD_10			10		// �˺ŵ�¼ ��֤
#define  CMD_20			20		// �˺�ע��
#define  CMD_30			30		// ���µ�¼ʱ��
#define  CMD_90			90		// ����

#define  CMD_100		100		// ��¼��Ϸ	��ȡ��ɫ����	
#define  CMD_200		200		// ѡ���ɫ	��ʼ��Ϸ
#define  CMD_300		300		// ������ɫ
#define  CMD_400		400		// ɾ����ɫ

#define  CMD_500		500		// ��ȡ��ɫ����
#define  CMD_600		600		// ��ҵ���
#define  CMD_610		610		// save base

#define  CMD_700		700		// other roledata
#define  CMD_710		710		// updata hp
#define  CMD_720		720		// updata mp
#define  CMD_730		730		// updata state
#define  CMD_740		740		// ����
#define  CMD_750		750		// �ȼ�
#define  CMD_760		760		// ����
#define  CMD_770		770		// ��������

#define  CMD_800		800		// �������߸���
#define  CMD_810		810		// ս��װ������
#define  CMD_820		820		// ��ȡ�����˵�ս��װ��
#define  CMD_830		830		// ��ȡ��������
#define  CMD_840		840		// ������
#define  CMD_850		850		// ����װ��
#define  CMD_860		860		// ����װ��
#define  CMD_870		870		// ���۵���
#define  CMD_880		880		// �ͻ����Լ��õģ�������װ��UI���¹�������
#define  CMD_890		890		// ��������

#define  CMD_900		900		// ��ʼ��������
#define  CMD_901		901		// ��ͼ
#define	 CMD_902		902		// ����·
#define  CMD_1000		1000	// �Լ��ƶ�
#define  CMD_1100		1100	// ��������ƶ�
#define  CMD_1200		1200	// �ٶȷ����仯
#define  CMD_2000		2000	// �ٶȷ����仯

#define  CMD_3000		3000	// ����
#define  CMD_3100		3100	// ���ܳ���
#define  CMD_3200		3200	// �����˺�

#define  CMD_4000		4000	// �������
#define  CMD_4100		4100	// �յ���Ҹ������ָ��
#define  CMD_4200		4200	// ��ҳɹ�����
#define  CMD_5000		5000	// buff

#define  CMD_6000       6000	// ���ش���
#define  CMD_6100       6100	// ��������
#define  CMD_6200       6200	// ɾ������
#define  CMD_6300       6300 

#define	 CMD_8000       8000	// ��������
#define	 CMD_8100       8100	// ���ﷵ��
#define	 CMD_8200       8200	// ��������
#define	 CMD_8300       8300	// ����״̬
#define	 CMD_8400       8400	// �����ƶ�
#define	 CMD_8500       8500	// �������
#define	 CMD_8700       8700	// ���＼���˺�
#define	 CMD_8800       8800	// ����BUFF

namespace app
{
	// const FString gate_ip = "127.0.0.1";
	const FString login_ip = "127.0.0.1";

	// const int gate_port = 14000;
	const int login_port = 15000;
}

#endif