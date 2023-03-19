#include "AppGame.h"
#include "UserManager.h"
namespace app
{
	AppGame* __AppGame = NULL;
	AppGame::AppGame()
	{
	}
	AppGame::~AppGame()
	{
	}
	void AppGame::onInit()
	{
	}

	//731 �������� ��ͼ�������
	void onStatusInfo(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex = 0;
		u64 memid = 0;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		auto user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onStatusInfo  user=null  %lld  line:%d\n", memid, __LINE__);
			return;
		}

		ts->read(c->ID, &user->role.base.status, sizeof(S_ROLE_BASE_STATUS));
	}
	void onExp(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex = 0;
		u64 memid;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		auto user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onExp  user=null  %lld  line:%d\n", memid, __LINE__);
			return;
		}
		ts->read(c->ID, user->role.base.exp.currexp);
	}
	//750 ����������������
	void onLevelUP(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex = 0;
		u64 memid;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);

		auto user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onLevelUP  user=null  %lld  line:%d\n", memid, __LINE__);
			return;
		}
		ts->read(c->ID, &user->role.base.exp, sizeof(S_ROLE_BASE_EXP));
		//save
		LOG_MESSAGE("onLevelUP 750  %lld  line:%d\n", memid, __LINE__);

	}

	//760	���¾���
	void onEconomy(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex = 0;
		u64 memid = 0;
		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);
		auto user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onEconomy  user=null  %lld  line:%d\n", memid, __LINE__);
			return;
		}
		ts->read(c->ID, &user->role.base.econ, sizeof(S_ROLE_BASE_ECON));
		LOG_MESSAGE("onEconomy 760  %lld  line:%d\n", memid, __LINE__);
	}
	//800 ���汳������
	void onBagProp(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32 userindex = 0;
		u64 memid = 0;
		S_ROLE_PROP prop;//����
		u8   bagpos = 0;
		bool iscount = false;
		u16   count = 0;
		u32   propid = 0;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);
		ts->read(c->ID, bagpos); //�����е�λ��
		ts->read(c->ID, iscount);//ֻ�������� 

		if (iscount)
		{
			ts->read(c->ID, propid);
			ts->read(c->ID, count);
		}
		else
		{
			prop.reset();
			ts->read(c->ID, &prop, prop.sendSize());
		}

		auto user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onBagProp  user=null  %lld  line:%d\n", memid, __LINE__);
			return;
		}
		if (bagpos >= USER_MAX_BAG)
		{
			LOG_MESSAGE("onBagProp bagpos >= USER_MAX_BAG ...%d line:%d \n", bagpos, __LINE__);
			return;
		}

		S_ROLE_PROP* bag_prop = &user->role.stand.bag.bags[bagpos];
		u8 type = bag_prop->base.type;
		if (iscount)
		{
			if (bag_prop->base.id != propid && count > 0)
			{
				return;
			}
			bag_prop->base.count = count;
			if (count == 0) bag_prop->reset();
		}
		else
		{
			memcpy(bag_prop, &prop, sizeof(S_ROLE_PROP));
		}
	}

	//810 ����ս��װ������ ���ϻ�������
	void onBagCompat(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		u32  userindex = 0;
		u64  memid;
		u8   kind = 0;//0��  1��   2װ���佻��
		u8   bagpos = 0;
		u8   equippos = 0;

		ts->read(c->ID, userindex);
		ts->read(c->ID, memid);
		ts->read(c->ID, kind);
		ts->read(c->ID, bagpos);
		ts->read(c->ID, equippos);

		auto user = __UserManager->findUser(userindex, memid);
		if (user == nullptr)
		{
			LOG_MESSAGE("onBagCompat  user=null  %lld  line:%d\n", memid, __LINE__);
			return;
		}
		if (bagpos >= USER_MAX_BAG || (kind == 2 && bagpos >= USER_MAX_EQUIP))
		{
			LOG_MESSAGE("onBagCompat bagpos >= USER_MAX_BAG ...%d line:%d \n", bagpos, __LINE__);
			return;
		}
		if (equippos >= USER_MAX_EQUIP)
		{
			LOG_MESSAGE("onBagCompat equippos >= USER_MAX_EQUIP ...%d line:%d \n", equippos, __LINE__);
			return;
		}
		S_ROLE_PROP* bag_prop = nullptr;
		if (kind == 2)
		{
			bag_prop = &user->role.stand.combat.equip[bagpos];
		}
		else
		{
			bag_prop = &user->role.stand.bag.bags[bagpos];			//������ĵ��� 
		}
		S_ROLE_PROP* equip_prop = &user->role.stand.combat.equip[equippos];	//�������ϵĵ���

		switch (kind)
		{
		case 0:// ��װ��
		case 2:// װ������
		{
			S_ROLE_PROP temp;
			temp.reset();
			memcpy(&temp, equip_prop, sizeof(S_ROLE_PROP));
			memcpy(equip_prop, bag_prop, sizeof(S_ROLE_PROP));
			memcpy(bag_prop, &temp, sizeof(S_ROLE_PROP));
		}
		break;
		case 1://��װ�� 
			memcpy(bag_prop, equip_prop, sizeof(S_ROLE_PROP));
			equip_prop->reset();
			break;
		break;
		}
	}

	bool AppGame::onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppLogin err...line:%d \n", __LINE__);
			return false;
		}

		switch (cmd)
		{
		case CMD_731:onStatusInfo(ts, c);   break;
		case CMD_740:onExp(ts, c);		break;
		case CMD_750:onLevelUP(ts, c);   break;//���������ڴ�����
		case CMD_760:onEconomy(ts, c);		break;
		case CMD_800:onBagProp(ts, c);   break;//������������
 		case CMD_810:onBagCompat(ts, c); break;//ս��װ����������
// 
// 		case CMD_921:onWareHouse_Gold(ts, c); break;
// 		case CMD_931:onWareHouse_Prop(ts, c); break;
// 
// 		case CMD_9110:onMainTask_Update(ts, c); break;
		}

		return true;
	}

	bool AppGame::onDBCommand(void* buff, const u16 cmd)
	{
		return true;
	}

}