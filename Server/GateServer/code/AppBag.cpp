
#include  "AppBag.h"
#include  "APPGlobal.h"
#include  "UserManager.h"
#include  "WorldData.h"

#include  "CommandData.h"

namespace app
{
	IContainer*  __AppBag;


	AppBag::AppBag()
	{
	}

	AppBag::~AppBag()
	{
	}

	//820 ��ȡ�������ս��װ��
	void onGetCompat(net::ITCPServer * ts, net::S_CLIENT_BASE * c)
	{
		S_COMBAT_DATA  data;
		ts->read(c->ID, data.userindex);

		S_USER_GATE* user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, 4001);
			return;
		}
		//������ ��ȡս��װ��ʱ��  3���� ���� ���ز����������
		//���ʱ������Լ�����
		s32 ftime = time(NULL) - user->temp_GetCompatTime;
		if (ftime < USER_GETCOMPAT_TIME)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, 4002);
			return;
		}
		user->temp_GetCompatTime = time(NULL);

		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, 4003);
			return;
		}

		//���͸���Ϸ������
		tcp->begin(CMD_820);
		tcp->sss(&data, sizeof(S_COMBAT_DATA));
		tcp->end();
	}


	//830 ���뱳������
	void onBag_Get(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_BAG_DATA  data;

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, 4001);
			return;
		}

		//������ ��ȡ����ʱ��  10���� ���� ���ز����������
		//���ʱ������Լ�����
		s32 ftime = time(NULL) - user->temp_GetBagTime;
		if (ftime < USER_GETBAG_TIME)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, 4002);
			return;
		}
		user->temp_GetBagTime = time(NULL);

		//��������
		data.userindex = user->userindex;
		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, 4003);
			return;
		}

		//���͸���Ϸ������
		tcp->begin(CMD_830);
		tcp->sss(&data, sizeof(S_BAG_DATA));
		tcp->end();

	}
	//840 ���������� ��Ҫ�Ǻϲ���������
	void onBag_Combine(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_BAG_DATA  data;

		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, 4001);
			return;
		}
		//������ ��ȡ����ʱ��  10���� ���� ���ز����������
		//���ʱ������Լ�����
		s32 ftime = time(NULL) - user->temp_GetBagTime;
		if (ftime < USER_GETBAG_TIME)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, 4002);
			return;
		}
		user->temp_GetBagTime = time(NULL);

		data.userindex = user->userindex;
		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, 4003);
			return;
		}

		//���͸���Ϸ������
		tcp->begin(CMD_830);
		tcp->sss(&data, sizeof(S_BAG_DATA));
		tcp->end();
	}
	//850 ����װ��
	//860 ����װ��
	//870 �������߳���
	void onBag_EquipUpDown(net::ITCPServer* ts, net::S_CLIENT_BASE* c, u16 cmd)
	{
		S_EQUIPUPDOWN_DATA  data;
		ts->read(c->ID, data.pos);//��������ս��װ���е�λ��
		ts->read(c->ID, data.propid);
		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 4001);
			return;
		}
		data.userindex = user->userindex;
		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, 4003);
			return;
		}

		//���͸���Ϸ������
		tcp->begin(cmd);
		tcp->sss(&data, sizeof(S_EQUIPUPDOWN_DATA));
		tcp->end();
	}

	void onBag_EquipSwap(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_BAGEQUIPSWAP_DATA data;
		ts->read(c->ID, data.pos1);//��������ս��װ���е�λ��
		ts->read(c->ID, data.propid1);
		ts->read(c->ID, data.pos2);//��������ս��װ���е�λ��
		ts->read(c->ID, data.propid2);
		auto user = __UserManager->findUser(c->ID, c->memid);
		if (user == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_890, 4001);
			return;
		}
		data.userindex = user->userindex;
		data.user_connectid = c->ID;
		data.memid = user->memid;

		auto tcp = user->tcpGame;
		if (tcp == NULL)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_890, 4002);
			return;
		}
		if (tcp->getData()->state < func::S_CONNECT_SECURE)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_890, 4003);
			return;
		}

		//���͸���Ϸ������
		tcp->begin(CMD_890);
		tcp->sss(&data, sizeof(S_BAGEQUIPSWAP_DATA));
		tcp->end();
	}

	bool AppBag::onServerCommand(net::ITCPServer * ts, net::S_CLIENT_BASE * c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppLogin err...line:%d \n", __LINE__);
			return false;
		}

		if (c->clientType != func::S_TYPE_USER) return false;
		if (c->state != func::S_LOGIN) return false;

		switch (cmd)
		{
			case CMD_820:onGetCompat(ts, c);			break;//��ȡ�������ս��װ������
			case CMD_830:onBag_Get(ts, c);				break;//��ȡ��������
			case CMD_840:onBag_Combine(ts, c);			break;//����������
			case CMD_850:onBag_EquipUpDown(ts, c, cmd); break;//����װ��
			case CMD_860:onBag_EquipUpDown(ts, c, cmd); break;//����װ��
			case CMD_870:onBag_EquipUpDown(ts, c, cmd); break;//���߳���
			case CMD_890:onBag_EquipSwap(ts, c);		break;
			break;
		}
		return true;
	}

	//***************************************************************************
	//***************************************************************************
	//***************************************************************************
	//���±�������
	void onCMD_800(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		u8 bagpos = 0;
		bool iscount = false;
		u16 count = 0;
		S_ROLE_PROP prop;
		prop.reset();
	
		tc->read(connectid);
		tc->read(memid);
		tc->read(bagpos);
		tc->read(iscount);
		if (iscount)
		{
			tc->read(count);
		}
		else
		{
			tc->read(&prop, prop.sendSize());
		}
	
		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", connectid, memid, __LINE__);
			return;
		}
	
		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_800);
		__TcpServer->sss(c->ID, bagpos);
		__TcpServer->sss(c->ID, iscount);
		if (iscount)
		{
			__TcpServer->sss(c->ID, count);
		}
		else
		{
			__TcpServer->sss(c->ID, &prop, prop.sendSize());
		}
		__TcpServer->end(c->ID);
	
	}
	//��ȡս��װ��
	void onCMD_820(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_COMBAT_DATA data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_COMBAT_DATA));
		
		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
	
		//1 ��֤�ǲ��Ǵ���
		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_820, childcmd);
			return;
		}
	
		//ֻ����Ч����
		u8 num = 0;
		tc->read(num);
		__TcpServer->begin(c->ID, CMD_820);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, data.userindex);
		__TcpServer->sss(c->ID, num);
		if (num > 0)
		{
			for (u8 i = 0; i < num; i++)
			{
				u8 bagpos = 0;
				app::S_ROLE_PROP prop;
				prop.reset();
				tc->read(bagpos);// ս��װ���е�λ��
				tc->read(&prop, prop.sendSize());
				__TcpServer->sss(c->ID, bagpos);
				__TcpServer->sss(c->ID, &prop, prop.sendSize());
			}
		}
	
		__TcpServer->end(c->ID);
	}
	//��ȡ����ȫ������
	void onCMD_830(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_BAG_DATA data;
		u8 bagnum = 0;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_BAG_DATA));
		tc->read(bagnum);
	
		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
	
		//1 ��֤�ǲ��Ǵ���
		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_830, childcmd);
			return;
		}
	
		//2 ���� ��������
		u8 num = 0;
		tc->read(num);
		__TcpServer->begin(c->ID, CMD_830);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, bagnum);
		__TcpServer->sss(c->ID, num);
		if (num > 0)
		{
			for (u8 i = 0; i < num; i++)
			{
				u8 bagpos = 0;
				app::S_ROLE_PROP prop;
				prop.reset();
				tc->read(bagpos);//�����е�λ��
				tc->read(&prop, prop.sendSize());
	
				__TcpServer->sss(c->ID, bagpos);
				__TcpServer->sss(c->ID, &prop, prop.sendSize());
			}
		}
		__TcpServer->end(c->ID);
	}
	
	//��ȡ����ȫ������(����װ�����͵�)
	void onCMD_840(net::ITCPClient* tc)
	{
		u16 childcmd = 0;
		S_BAG_DATA data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_DROP_DATA));
	
		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
	
		//1 ��֤�ǲ��Ǵ���
		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, CMD_840, childcmd);
			return;
		}
	
		//2 ���� ��������
		u8 num = 0;
		tc->read(num);
		__TcpServer->begin(c->ID, CMD_840);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, num);
		if (num > 0)
		{
			for (u8 i = 0; i < num; i++)
			{
				u8 bagpos = 0;
				u8 count = 0;
				tc->read(bagpos);//�����е�λ��
				tc->read(count); //��ǰ����
	
				__TcpServer->sss(c->ID, bagpos);
				__TcpServer->sss(c->ID, count);
			}
		}
		__TcpServer->end(c->ID);
	}
	//810 ���ϻ�������װ��
	void onCMD_810(net::ITCPClient* tc)
	{
		u32 connectid = 0;
		u64 memid = 0;
		u8 kind = 0;
		u8 bagpos = 0;
		u8 equippos = 0;
	
		tc->read(connectid);
		tc->read(memid);
		tc->read(kind);
		tc->read(bagpos);
		tc->read(equippos);
	
		auto user = __UserManager->findUser_Connection(connectid, memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", connectid, memid, __LINE__);
			return;
		}
	
		auto c = user->connection;
		__TcpServer->begin(c->ID, CMD_810);
		__TcpServer->sss(c->ID, kind);
		__TcpServer->sss(c->ID, bagpos);
		__TcpServer->sss(c->ID, equippos);
		__TcpServer->end(c->ID);
	
	}
	//����װ�� �������� ����,��ȷ�Ĵ���װ����CMD_800
	void onCMD_850(net::ITCPClient* tc,u16 cmd)
	{
		u16 childcmd = 0;
		S_EQUIPUPDOWN_DATA data;
		tc->read(childcmd);
		tc->read(&data, sizeof(S_DROP_DATA));
	
		auto user = __UserManager->findUser_Connection(data.user_connectid, data.memid);
		if (user == NULL)
		{
			LOG_MESSAGE("AppBag user == NULL...%d-%lld line:%d \n", data.user_connectid, data.memid, __LINE__);
			return;
		}
	
		//1 ��֤�ǲ��Ǵ���
		auto c = user->connection;
		if (childcmd != 0)
		{
			sendErrInfo(__TcpServer, c->ID, cmd, childcmd);
			return;
		}
	}
	bool AppBag::onClientCommand(net::ITCPClient * tc, const u16 cmd)
	{
		if (tc->getData()->serverType != func::S_TYPE_GAME) return false;
	
		switch (cmd)
		{
		case CMD_800:  onCMD_800(tc);  break;//���±����еĵ���
		case CMD_820:  onCMD_820(tc);  break;//��ȡս��װ��
		case CMD_830:  onCMD_830(tc);  break;//��ȡ��������
		case CMD_840:  onCMD_840(tc);  break;//����������
		case CMD_810:  onCMD_810(tc);  break;//���ϻ������� ��������
		case CMD_850: 
		case CMD_860:
		case CMD_870:
			onCMD_850(tc,cmd);  break;//���ϻ������´��󷵻� ���۽��
		}
		return true;
	}
}