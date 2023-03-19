#ifndef  ____COMMANDDATA_H
#define  ____COMMANDDATA_H

#include "UserData.h"
#include "IDefine_Udp.h"

#define  CMD_10			10		// �˺ŵ�¼ ��֤
#define  CMD_20			20		// �˺�ע��
#define  CMD_30			30		// ���µ�¼ʱ��
#define  CMD_40         40		//������������
#define  CMD_90			90		// ����

#define  CMD_100		100		// ��¼��Ϸ	��ȡ��ɫ����	
#define  CMD_200		200		// ѡ���ɫ	��ʼ��Ϸ
#define  CMD_300		300		// ������ɫ
#define  CMD_400		400		// ɾ����ɫ

#define  CMD_500		500		// ��ȡ��ɫ����
#define  CMD_600		600		// ��ҵ���
#define  CMD_601        601     // ��ҵ��� �������˺������mini��ɫ����
#define  CMD_602        602     // DB��game�Ͽ����� ֪ͨgate
#define  CMD_610		610		// save base

#define  CMD_700		700		// other roledata
#define  CMD_710		710		// updata hp
#define  CMD_720		720		// updata mp
#define  CMD_730		730		// updata state
#define  CMD_731        731     // ״̬�����꣩ 
#define  CMD_740		740		// ����
#define  CMD_750		750		// �ȼ�
#define  CMD_760		760		// ����
#define  CMD_770		770		// ��������


#define  CMD_800		800		// �������߱���
#define  CMD_810		810		// ս��װ������
#define  CMD_820		820		// ��ȡս��װ�����Լ��ĺ����˶��������ȡ��
#define  CMD_830		830		// ��ȡ��������
#define  CMD_840		840		// ������
#define  CMD_850		850		// ����װ��
#define  CMD_860		860		// ����װ��
#define  CMD_870		870		// ���۵���
#define	 CMD_890		890		// ����װ������

#define  CMD_900		900		// ��ʼ��������
#define  CMD_901		901		// �л���ͼ
#define  CMD_902		902		// ����

#define  CMD_1000		1000	// �Լ��ƶ�
#define  CMD_1100		1100	// ��������ƶ�
#define  CMD_1200		1200	// �ٶȷ����仯
#define  CMD_2000		2000	// ����

#define  CMD_3000		3000	// ����
#define  CMD_3100		3100	// ���ܳ���
#define  CMD_3200		3200	// �����˺�

#define  CMD_4000		4000	// �������
#define  CMD_4100		4100	// �յ���Ҹ������ָ��
#define  CMD_4200		4200	// ��ҳɹ�����
#define  CMD_5000		5000	// buff

#define  CMD_6000		6000	// ʰȡ����
#define  CMD_6100		6100 	// ������Ʒ
#define  CMD_6200		6200 	// ����ɾ��
#define  CMD_6300		6300	// ����

#define  CMD_7000		7000	// ���-��������
#define  CMD_7010		7010	// ���-�������
#define  CMD_7020		7020	// ���-ͬ��������
#define  CMD_7030		7030	// ���-�ܾ��������
#define  CMD_7040		7040	// ���-�����뿪
#define  CMD_7050		7050	// ���-�ӳ�����
#define  CMD_7060		7060	// ���-����������
#define  CMD_7070		7070	// ���-������� �ӳ�ͬ��
#define  CMD_7080		7080	// ���-������� �ӳ��ܾ�
#define  CMD_7090		7090	// ���-�ӳ�ת��
#define  CMD_7100		7100	// ���¶����Ա����
#define  CMD_7110		7110	// �㲥xx��Ϊ�˶ӳ�
#define  CMD_7120		7120	// �Լ������˶���

#define	 CMD_7200		7200	// �������˸���
#define	 CMD_7300		7300	// �������˸���
#define	 CMD_7301		7301	// ���븱�� 
#define	 CMD_7400		7400	// �뿪����
#define	 CMD_7401		7401	// �����ѽ�ɢ

#define  CMD_7500		7500	// ���� ����Ϸ������ �������
#define  CMD_7600		7600	// ���� ����Ϸ������ ��ҵĶ�������

#define	 CMD_8000		8000	// �����������
#define	 CMD_8100		8100	// ���ﷵ��
#define	 CMD_8200		8200	// ��������
#define	 CMD_8300		8300	// ����״̬
#define	 CMD_8400		8400	// �����ƶ�
#define	 CMD_8500		8500	// �������
#define	 CMD_8700		8700	// ���＼���˺�
#define	 CMD_8800		8800	// ����BUFF

namespace app
{
#pragma pack(push, packing)
#pragma pack(1)
	struct S_LOGIN_GAME
	{
		this_constructor(S_LOGIN_GAME);

		u8				line;//ѡ�����·
		u32				ServerConnectID;			// ���ķ������ϵ�����ID����ʵҲ�����������������ϵ�ID��
		u32				ServerClientID;				// ���ķ������ϵĿͻ���ID����ʵҲ�����������������ϵ�ID��
		u32				UserConnectID;				// ���ط������ϵ�����ID����ʵҲ�����û����������ϵ�ID��
		u32				UserClientID;				// ���ط������ϵĿͻ���ID����ʵҲ�����û����������ϵ�ID��
		char			name[USER_MAX_MEMBER];		// �˺�
		char			key[USER_MAX_KEY];			// ��Կ
	};

	struct S_COMMAND_BASE
	{
		u32				server_connectid;			// �������ϵ�����ID
		u32				server_clientid;			// ���ķ������ϵĿͻ���ID
		u32				user_connectid;				// ���ط������ϵ�����ID
		u64				memid;						// �˺�id
	};

	struct S_COMMAND_GATEBASE
	{
		u32				user_connectid;				// ���ط������ϵ�����ID
		u64				memid;						// �˺�id
	};

	struct S_SELECT_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_SELECT_ROLE);

		u8				roleindex;					// ��ɫ����
		u32				userindex;					// �������λ�ã�DB��������ֵ
		u32				mapid;						// ��ͼid��DB��������ֵ
		inline void reset() { memset(this, 0, sizeof(S_SELECT_ROLE)); };
	};

	struct S_CREATE_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_CREATE_ROLE);

		u64				roleid;						// ��ɫID
		u8				roleindex;					// ��ɫ����
		u8				job;						// ְҵ
		u8				sex;						// �Ա�
		char			nick[USER_MAX_NICK];		// �ǳ�
	};

	struct S_DELETE_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_DELETE_ROLE);

		u64				roleid;						// ��ɫID
		u8				roleindex;					// ��ɫ����
	};

	struct S_LOAD_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_LOAD_ROLE);

		u16 cmd;
		u32 userindex;								// DB�������ϻ���������������ֵ
		u32 mapid;	
		u8  line;
		inline void reset() { memset(this, 0, sizeof(S_LOAD_ROLE)); };

	};

	// ������������
	struct S_ENTRY_BASE : S_COMMAND_BASE
	{
		this_constructor(S_ENTRY_BASE);

		u32 userindex;
		inline void reset() { memset(this, 0, sizeof(S_ENTRY_BASE)); };
	};

	struct S_UPDATE_VALUE : S_COMMAND_BASE
	{
		this_constructor(S_UPDATE_VALUE);

		s32 value;
		u32 targetindex;
		inline void reset() { memset(this, 0, sizeof(S_UPDATE_VALUE)); };
	};

	struct S_GETROLE_DATA : S_COMMAND_BASE
	{
		this_constructor(S_GETROLE_DATA);

		u32 targetindex;		//��Ҫ��ȡ���������ֵ
		inline void reset() { memset(this, 0, sizeof(S_GETROLE_DATA)); };
	};

	struct S_MOVE_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_MOVE_ROLE);

		u32 userindex;
		s16 face;
		s32 speed;		
		S_VECTOR3 curpos;
		S_VECTOR3 targetpos;
		inline void reset() { memset(this, 0, sizeof(S_MOVE_ROLE)); };
	};

	struct S_SKILL_ROLE : S_COMMAND_BASE
	{
		this_constructor(S_SKILL_ROLE);

		u32	userindex;				// ʹ�ü��ܵ��������
		u8	skillindex;				// ��������
		u8	lock_type;				// ��������
		u32	lock_index;			// ����index
		S_VECTOR3 targetpos;		// Ŀ���
		inline void reset() { memset(this, 0, sizeof(S_SKILL_ROLE)); };
	};

	//��ȡ������ ս��װ��
	struct S_COMBAT_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_COMBAT_DATA)

			u32  userindex;
	};
	//��ȡ��������
	struct S_BAG_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_BAG_DATA)

			u32  userindex;
	};

	//���� ����װ�� ���߳��� 
	struct S_EQUIPUPDOWN_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_EQUIPUPDOWN_DATA)

		u32  userindex;
		u8   pos;			// ��������ս��װ���е�λ��
		u32  propid;		// ����ID
	};
	// ��������
	struct S_BAGEQUIPSWAP_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_BAGEQUIPSWAP_DATA)

		u32  userindex;
		u8   pos1;			// ��������ս��װ���е�λ��
		u32  propid1;		// ����ID
		u8   pos2;			// ��������ս��װ���е�λ��
		u32  propid2;		// ����ID
	};

	// *******************************************************************************
	// *******************************************************************************
	// *******************************************************************************
	// ����
	//�������� 7000
	struct S_TEAM_CREATE :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_CREATE)

			u32  userindex;
		s32  teamindex;
		u8   mateindex;
		bool isleader;
	};
	//����xxx������� 7010
	struct S_TEAM_PLEASE :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_PLEASE)

			u32  userindex;
		s32  other_userindex;
		s64  other_memid;
		char nick[USER_MAX_NICK];
	};

	//7020 ͬ�����xxx�Ķ���
	struct S_TEAM_AGREE :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_AGREE)

			u32  userindex;
		u32  leaderindex;//�ӳ�����
		s64  leadermemid;//�ӳ��˺�id
	};
	//7040 xxx�뿪����
	struct S_TEAM_LEAVE :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_LEAVE)

			u32  userindex;
		u8   mateindex;
	};
	//7050 �ӳ�����
	struct S_TEAM_KILL :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_KILL)

			u32  userindex;
		u8   mateindex;
	};
	//7060 �������xx����
	struct S_TEAM_REQUEST :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_REQUEST)

			u32  userindex;
		s32  otherindex;
		s64  othermemid;
	};
	//7070 �������xx���� �ӳ�ͬ��
	struct S_TEAM_LEADER :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_LEADER)

			u32  userindex;
		s32  joinindex; //����������
		s64  joinmemid;
	};

	//7090 ת�öӳ�
	struct S_TEAM_NEWLEADER :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_NEWLEADER)

			u32  userindex;
		u8   mateindex;
	};

	//7100 �����Ա��Ϣ
	struct S_TEAM_INFO :S_COMMAND_GATEBASE
	{
		this_constructor(S_TEAM_INFO)
			u32  userindex;

		u32  other_index;//��Ϸ����
		s64  other_memid;//�˺�id
		s32  teamindex; //��������
		u8   materindex; //��Ա����
		u8   sex;//�Ա�
		u8   job;//ְҵ
		u16  level;//�ȼ�
		bool isleader;
		char nick[USER_MAX_NICK];//�ǳ�
	};

	struct S_REBORN_ROLE : S_COMMAND_GATEBASE
	{
		this_constructor(S_REBORN_ROLE);

		u8	kind;				// ��������
		s32 userindex;
		// S_VECTOR3 targetpos;		// Ŀ���
		inline void reset() { memset(this, 0, sizeof(S_REBORN_ROLE)); };
	};

	struct S_ROBOT_DATA : S_COMMAND_GATEBASE
	{
		this_constructor(S_ROBOT_DATA);
		u32 robotindex;
	};

	struct S_DROP_DATA :S_COMMAND_GATEBASE
	{
		this_constructor(S_DROP_DATA)

		u32  userindex;
		u32  dropindex;
		s32  dropid;
	};
	//�л���ͼ
	struct S_CHANGEMAP_BASE :S_COMMAND_GATEBASE
	{
		this_constructor(S_CHANGEMAP_BASE)

		u8   line;
		u32  userindex; //���Ψһ����
		u32  mapid;     //��Ҫ�л����ĵ�ͼID

		inline void reset() { memset(this, 0, sizeof(S_CHANGEMAP_BASE)); }

	};

	struct  S_SKILL_3000 :public func::S_DATA_HEAD
	{
		u8 skillindex;
		u8 lock_type;
		s32 lock_index;
		S_VECTOR3   pos;
	};
	struct  S_SKILL_3100 :public func::S_DATA_HEAD
	{
		s32 userindex;
		s32 skillid;
		u8  level;
		u8 skillindex;
		u8  lock_type;//��������
		s32 lock_index;
		S_VECTOR3   pos;
	};

#pragma pack(pop, packing)
}

#endif