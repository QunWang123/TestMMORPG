
#include  "AppBag.h"
#include  "APPGlobal.h"
#include  "UserManager.h"
#include  "WorldData.h"
#include  "WorldScript.h"
#include  "CommandData.h"
#include  "WorldBC.h"
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
		S_COMBAT_DATA data;
		ts->read(c->ID, &data, sizeof(S_COMBAT_DATA));
		
		//1�����һ�ȡװ�������
		auto user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_820, 3001, &data, sizeof(S_COMBAT_DATA));
			return;
		}
		if (!user->mem.isT())
		{
			sendErrInfo(ts, c->ID, CMD_820, 3002, &data, sizeof(S_COMBAT_DATA));
			return;
		}

		//���ظ����ս��װ������
		u8 num = 0;
		for (u32 i = 0; i < USER_MAX_EQUIP; i++)
		{
			auto e = &user->role.stand.combat.equip[i];
			if (!e->IsT()) continue;
			num++;
		}

		//���ظ������ϸ����
		__TcpServer->begin(c->ID, CMD_820);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, &data, sizeof(S_COMBAT_DATA));
		__TcpServer->sss(c->ID, num);
		for (u8 i = 0; i < USER_MAX_EQUIP; i++)
		{
			auto e = &user->role.stand.combat.equip[i];
			if (!e->IsT()) continue;
			__TcpServer->sss(c->ID, i);//ս��װ���е�λ��
			__TcpServer->sss(c->ID, e, e->sendSize());
		}
		__TcpServer->end(c->ID);

		// ˳��ѹ�������Ҳ��һ��
		__TcpServer->begin(c->ID, CMD_770);
		__TcpServer->sss(c->ID, data.user_connectid);
		__TcpServer->sss(c->ID, data.memid);
		__TcpServer->sss(c->ID, user->node.index);
		__TcpServer->sss(c->ID, &user->atk.total, sizeof(S_SPRITE_ATTACK));
		__TcpServer->end(c->ID);
	}

	//830 ���뱳������
	void onBag_Get(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_BAG_DATA  data;
		ts->read(c->ID, &data, sizeof(S_BAG_DATA));

		//1������ʹ�ü������
		auto user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_830, 3001, &data, sizeof(S_BAG_DATA));
			return;
		}
		if (!user->mem.isT())
		{
			sendErrInfo(ts, c->ID, CMD_830, 3002, &data, sizeof(S_BAG_DATA));
			return;
		}
		//���ظ���ұ�������
		u8 num = 0;
		for (u32 i = 0; i < user->role.stand.bag.num; i++)
		{
			if (i >= USER_MAX_BAG) continue;
			auto e = &user->role.stand.bag.bags[i];
			if (!e->IsT()) continue;
			num++;
		}

		//ֻ����Ч����
		__TcpServer->begin(c->ID, CMD_830);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, &data, sizeof(S_BAG_DATA));
		__TcpServer->sss(c->ID, user->role.stand.bag.num);
		__TcpServer->sss(c->ID, num);
		for (u8 i = 0; i < user->role.stand.bag.num; i++)
		{
			if (i >= USER_MAX_BAG) continue;
			auto e = &user->role.stand.bag.bags[i];
			if (!e->IsT()) continue;
			__TcpServer->sss(c->ID, i);//�����е�λ��
			__TcpServer->sss(c->ID, e, e->sendSize());//��������
		}

		__TcpServer->end(c->ID);
	}

	//840 ���������� ��Ҫ�Ǻϲ���������
	void onBag_Combine(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_BAG_DATA  data;
		ts->read(c->ID, &data, sizeof(S_BAG_DATA));

		//1������ʹ�ü������
		auto user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_840, 3001, &data, sizeof(S_BAG_DATA));
			return;
		}
		if (!user->mem.isT())
		{
			sendErrInfo(ts, c->ID, CMD_840, 3002, &data, sizeof(S_BAG_DATA));
			return;
		}

		u8 len = user->role.stand.bag.num;
		if (len > USER_MAX_BAG) len = USER_MAX_BAG;
		std::queue<u8> freepos;
		s32 curcount = 0;
		s32 maxcount = 0;
		// bagpos������¼ԭ��ÿһ������λ��ȥ���ĸ��µ�λ��
		s8 bagpos[USER_MAX_BAG];
		for (u8 i = 0; i < USER_MAX_BAG; i++)
		{
			bagpos[i] = i;
		}

		for (u8 i = 0; i < len; i++)
		{
			S_ROLE_PROP* prop = &user->role.stand.bag.bags[i];
			if (!prop->IsT())
			{
				// �������û����(������ԭ������)
				// 1.������û����			ֱ�Ӽ�����ж���
				// 2.ԭ���У��������		�ñ��һ��
				freepos.push(i);
				if (bagpos[i] != i)
				{
					// ˵�����λ�ñ����
				}
				continue;
			}
			if (prop->base.type != EPT_CONSUME)
			{
				// ���Ǹ��������ӣ����ɵ���
				if (!freepos.empty())
				{
					u8 tmpfreepos = freepos.front();
					freepos.pop();

					S_ROLE_PROP* tmpprop = &user->role.stand.bag.bags[tmpfreepos];

					memcpy(tmpprop, prop, sizeof(S_ROLE_PROP));
					prop->reset();

					TS_Broadcast::db_SendUpdateProp(user, tmpfreepos, false);
					TS_Broadcast::db_SendUpdateProp(user, i, true);

					bagpos[i] = tmpfreepos;
					freepos.push(i);
				}
				continue;
			}
			// ��Ϊ�գ���������Ʒ
			auto sss = (script::SCRIPT_PROP*)prop->temp.script_prop;
			if (sss == NULL) continue;

			maxcount = sss->maxcount;
			bool isadd = false;
			if (prop->base.count < maxcount)
			{
				curcount = 0;
				for (u8 index = i + 1; index < len; index++)
				{
					S_ROLE_PROP* downprop = &user->role.stand.bag.bags[index];
					if (!prop->isSame(downprop))          continue;
					if (downprop->base.count >= maxcount) continue;
					if (downprop->base.count == 0)        continue;

					isadd = true;

					// ����-1˵����ֵ���Ĺ�
					bagpos[index] = -1;
					curcount = prop->base.count + downprop->base.count;

					//1���������ޣ�
					if (curcount >= maxcount)
					{
						prop->base.count = maxcount;
						curcount = curcount - maxcount;
						if (curcount == 0) downprop->reset();
						else if (curcount > 0) downprop->base.count = curcount;

						//���µ�������
						TS_Broadcast::db_SendUpdateProp(user, index, true);
						break;
					}

					prop->base.count = curcount;
					downprop->reset();
					//���µ�������
					TS_Broadcast::db_SendUpdateProp(user, index, true);
				}
			}
			if (!freepos.empty())
			{
				// ǰ���и���λ��
				u8 tmpfreepos = freepos.front();
				freepos.pop();
				
				// �ƶ���ȥ
				S_ROLE_PROP* tmpprop = &user->role.stand.bag.bags[tmpfreepos];
				memcpy(tmpprop, tmpprop, sizeof(S_ROLE_PROP));
				memcpy(tmpprop, prop, sizeof(S_ROLE_PROP));
				prop->reset();
				TS_Broadcast::db_SendUpdateProp(user, tmpfreepos, false);
				TS_Broadcast::db_SendUpdateProp(user, i, true);

				freepos.push(i);
				bagpos[i] = tmpfreepos;
				continue;
			}
			// ����˵��i֮ǰ��û�пո���
			if (!isadd) continue;
			// ����˵��i֮ǰ��û�пո��ӵ�i����ֵ����
			bagpos[i] = -1;
			TS_Broadcast::db_SendUpdateProp(user, i, true);
		}
		u8 changecount = 0;
		for (u8 i = 0; i < USER_MAX_BAG; i++)
		{
			if (bagpos[i] != i)
			{
				changecount += 1;
			}
		}
		__TcpServer->begin(c->ID, CMD_840);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, &data, sizeof(S_BAG_DATA));
		__TcpServer->sss(c->ID, changecount);// �����仯������
		for (u8 i = 0; i < USER_MAX_BAG; i++)
		{
			if (bagpos[i] != i)
			{
				__TcpServer->sss(c->ID, i);
				__TcpServer->sss(c->ID, bagpos[i]);
				S_ROLE_PROP* prop = nullptr;
				if (bagpos[i] == -1)
				{
					prop = &user->role.stand.bag.bags[i];
				}
				else
				{
					prop = &user->role.stand.bag.bags[bagpos[i]];
				}
				__TcpServer->sss(c->ID, prop->base.count);
			}
		}
		__TcpServer->end(c->ID);
	}

	//850 ����װ��
	void onBag_EquipUp(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_EQUIPUPDOWN_DATA  data;
		ts->read(c->ID, &data, sizeof(S_EQUIPUPDOWN_DATA));

		//1������ʹ�ü������
		auto user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_850, 3001, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		if (!user->mem.isT())
		{
			sendErrInfo(ts, c->ID, CMD_850, 3002, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//1��λ�ô���
		if (data.pos >= USER_MAX_BAG)
		{
			sendErrInfo(ts, c->ID, CMD_850, 3003, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//2�����ͻ���ID����
		S_ROLE_PROP* prop = &user->role.stand.bag.bags[data.pos];
		if (prop->base.type != EPT_EQUIP || prop->base.id != data.propid)
		{
			sendErrInfo(ts, c->ID, CMD_850, 3004, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//3���ű�����
		script::SCRIPT_PROP* s_prop = (script::SCRIPT_PROP*)prop->temp.script_prop;
		if (s_prop == nullptr)
		{
			LOG_MESSAGE("onBag_EquipUp_850 err:script == NULL, line:%d\n", __LINE__);
			sendErrInfo(ts, c->ID, CMD_850, 3004, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		if (s_prop->equip.id < 10000)
		{
			sendErrInfo(ts, c->ID, CMD_850, 3004, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//4��ְҵ��֤
		if ((user->role.base.innate.job & s_prop->equip.job) == 0)
		{
			sendErrInfo(ts, c->ID, CMD_850, 3005, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//5���ȼ���֤
		if (user->role.base.exp.level < s_prop->equip.level)
		{
			sendErrInfo(ts, c->ID, CMD_850, 3006, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//6��װ��������֤
		if (s_prop->equip.kind == EPE_UNKNOWN)
		{
			sendErrInfo(ts, c->ID, CMD_850, 3006, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		u8 pos = 0;

		switch (s_prop->equip.kind)
		{
		case EPE_WEQPON:		pos = ERP_WEQPON; break;
		case EPE_HEAD:			pos = ERP_HEAD; break;
		case EPE_CLOTH:			pos = ERP_CLOTH; break;
		case EPE_DECORATIVE:	pos = ERP_DECORATIVE; break;
		case EPE_SHOES:			pos = ERP_SHOES; break;
		case EPE_NECKLACE:		pos = ERP_NECKLACE; break;
		}

		S_ROLE_PROP* equip = NULL;
		switch (s_prop->equip.kind)
		{
		case EPE_RING: //��ָ
			pos = ERP_RING_LEFT;
			equip = &user->role.stand.combat.equip[ERP_RING_LEFT];
			if (equip->IsT())
			{
				// ���ָ�ж����ˣ������ҽ�ָ��û��
				S_ROLE_PROP* equip2 = &user->role.stand.combat.equip[ERP_RING_RIGHT];
				if (!equip2->IsT())
				{
					pos = ERP_RING_RIGHT;
					equip = equip2;
				}
			}
			break;
		case EPE_BANGLE: //����
			pos = ERP_BANGLE_LEFT;
			equip = &user->role.stand.combat.equip[ERP_BANGLE_LEFT];
			if (equip->IsT())
			{
				S_ROLE_PROP* equip2 = &user->role.stand.combat.equip[ERP_BANGLE_RIGHT];
				if (!equip2->IsT())
				{
					pos = ERP_BANGLE_RIGHT;
					equip = equip2;
				}
			}
			break;
		default:
			equip = &user->role.stand.combat.equip[pos];
			break;
		}

		// ����ط��ǽ������е�λ�ú�װ�����е�λ�û���
		S_ROLE_PROP temp;
		memcpy(&temp, equip, sizeof(S_ROLE_PROP));
		memcpy(equip, prop, sizeof(S_ROLE_PROP));
		memcpy(prop, &temp, sizeof(S_ROLE_PROP));

		
		user->updateAtk(true);
		TS_Broadcast::do_SendValue(CMD_710, user->node.index, user->role.base.life.hp, user);
		TS_Broadcast::do_SendValue(CMD_720, user->node.index, user->role.base.life.mp, user);
		TS_Broadcast::do_SendAtk(user, user);
		
		TS_Broadcast::db_SaveCompat(user, data.pos, pos, 0);
		TS_Broadcast::do_SendCompat(user, data.pos, pos, 0);
	}


	//860 ����װ��
	void onBag_EquipDown(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_EQUIPUPDOWN_DATA  data;
		ts->read(c->ID, &data, sizeof(S_EQUIPUPDOWN_DATA));

		//1������ʹ�ü������
		auto user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_860, 3001, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		if (!user->mem.isT())
		{
			sendErrInfo(ts, c->ID, CMD_860, 3002, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//1��λ�ô���
		if (data.pos >= ERP_COUNT)
		{
			sendErrInfo(ts, c->ID, CMD_860, 3003, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//2�����ͻ���ID����
		S_ROLE_PROP* prop_compat = &user->role.stand.combat.equip[data.pos];
		if (prop_compat->IsT() == false || prop_compat->base.id != data.propid)
		{
			sendErrInfo(ts, c->ID, CMD_860, 3004, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//3���ű�����
		script::SCRIPT_PROP* s_prop = (script::SCRIPT_PROP*)prop_compat->temp.script_prop;
		if (s_prop == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_860, 3004, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		if (s_prop->equip.id < 10000)
		{
			sendErrInfo(ts, c->ID, CMD_860, 3004, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}

		//4��������֤
		u8 bagpos = 0;
		S_ROLE_PROP* prop_bag = user->role.stand.bag.findEmpty(bagpos);
		if (prop_bag == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_860, 3005, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}

		memcpy(prop_bag, prop_compat, sizeof(S_ROLE_PROP));
		prop_compat->reset();

		
		user->updateAtk(true);
		TS_Broadcast::do_SendValue(CMD_710, user->node.index, user->role.base.life.hp, user);
		TS_Broadcast::do_SendValue(CMD_720, user->node.index, user->role.base.life.mp, user);
		TS_Broadcast::do_SendAtk(user, user);
		
		TS_Broadcast::db_SaveCompat(user, bagpos, data.pos, 1);
		TS_Broadcast::do_SendCompat(user, bagpos, data.pos, 1);
	}

	//870 ���۵���
	void onBag_Sell(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_EQUIPUPDOWN_DATA  data;
		ts->read(c->ID, &data, sizeof(S_EQUIPUPDOWN_DATA));

		//1������ʹ�ü������
		auto user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_870, 3001, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		if (!user->mem.isT())
		{
			sendErrInfo(ts, c->ID, CMD_870, 3002, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//1��λ�ô���
		if (data.pos >= USER_MAX_BAG)
		{
			sendErrInfo(ts, c->ID, CMD_870, 3003, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//2��ID����
		S_ROLE_PROP* prop = &user->role.stand.bag.bags[data.pos];
		if (prop->base.id != data.propid)
		{
			sendErrInfo(ts, c->ID, CMD_870, 3004, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		//3���ű�����
		script::SCRIPT_PROP* s_prop = (script::SCRIPT_PROP*)prop->temp.script_prop;
		if (s_prop == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_870, 3005, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		if (s_prop->equip.id < 10000)
		{
			sendErrInfo(ts, c->ID, CMD_870, 3005, &data, sizeof(S_EQUIPUPDOWN_DATA));
			return;
		}
		u32 gold = s_prop->price * prop->base.count;
		user->role.base.econ.gold += gold;
		prop->reset();
		//���µ�������
		TS_Broadcast::db_SendUpdateProp(user, data.pos, true);
		//���½��
		TS_Broadcast::do_SendUpdateProp(user, data.pos, true);
		TS_Broadcast::do_SendGold(user);
		TS_Broadcast::db_SaveGold(user);
	}

	// 890 ��������
	void onBagEquip_Swap(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_BAGEQUIPSWAP_DATA  data;
		ts->read(c->ID, &data, sizeof(S_BAGEQUIPSWAP_DATA));
		// ��֤��ɫ
		auto user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			sendErrInfo(ts, c->ID, CMD_890, 3001, &data, sizeof(S_BAGEQUIPSWAP_DATA));
			return;
		}
		if (!user->mem.isT())
		{
			sendErrInfo(ts, c->ID, CMD_890, 3002, &data, sizeof(S_BAGEQUIPSWAP_DATA));
			return;
		}
		// ���߲�����ͬһ��λ��
		if (data.pos1 == data.pos2 || data.pos1 < 0 || data.pos2 < 0)
		{
			sendErrInfo(ts, c->ID, CMD_890, 3003, &data, sizeof(S_BAGEQUIPSWAP_DATA));
			return;
		}
		// ��֤��һ������
		S_ROLE_PROP* prop1 = nullptr;
		if (data.pos1 < USER_MAX_BAG)
		{
			prop1 = &user->role.stand.bag.bags[data.pos1];
		}
		else if (data.pos1 < USER_MAX_BAG + ERP_COUNT)
		{
			prop1 = &user->role.stand.combat.equip[data.pos1 - USER_MAX_BAG];
		}

		if (!prop1 || !prop1->IsT() || prop1->base.id != data.propid1)
		{
			// ��ק��Ʒ����
			sendErrInfo(ts, c->ID, CMD_890, 3004, &data, sizeof(S_BAGEQUIPSWAP_DATA));
			return;
		}
		script::SCRIPT_PROP* s_prop1 = (script::SCRIPT_PROP*)prop1->temp.script_prop;
		if (s_prop1 == nullptr)
		{
			LOG_MESSAGE("onBagEquip_Swap err:script == NULL, line:%d\n", __LINE__);
			sendErrInfo(ts, c->ID, CMD_890, 3005, &data, sizeof(S_BAGEQUIPSWAP_DATA));
			return;
		}
		// ��֤�ڶ�������
		S_ROLE_PROP* prop2 = nullptr;
		if (data.pos2 < USER_MAX_BAG)
		{
			prop2 = &user->role.stand.bag.bags[data.pos2];
		}
		else if (data.pos2 < USER_MAX_BAG + ERP_COUNT)
		{
			prop2 = &user->role.stand.combat.equip[data.pos2 - USER_MAX_BAG];
		}

		if (!prop2 || prop2->base.id != data.propid2)
		{
			sendErrInfo(ts, c->ID, CMD_890, 3004, &data, sizeof(S_BAGEQUIPSWAP_DATA));
			return;
		}
		script::SCRIPT_PROP* s_prop2 = nullptr;
		if (prop2->IsT())
		{
			s_prop2 = (script::SCRIPT_PROP*)prop2->temp.script_prop;
			if (s_prop2 == nullptr)
			{
				LOG_MESSAGE("onBagEquip_Swap err:script == NULL, line:%d\n", __LINE__);
				sendErrInfo(ts, c->ID, CMD_890, 3005, &data, sizeof(S_BAGEQUIPSWAP_DATA));
				return;
			}
		}
		
		// ����������жϿ�֪��pos1һ�����ж�����pos2δ֪,��pos1!=pos2
		// ���������֣�����-���� ����-װ�� װ��-���� װ��-װ��
		// ����-������1.�Ƿ���Ե���											2.���ɵ��ӣ�ֱ�ӽ������У�
		// ����-װ�����ʹ�װ���߼�һ��
		// װ��-������1.������pos2�ж������Ҹ���λ�ý�����						2.û����������ֱ�ӽ���
		// װ��-װ����1.pos2��pos1��װ��λ��ͬ���ͣ�ֱ�ӽ������ɣ�				2.���߲�ͬ���ͣ�������
		if (data.pos1 < USER_MAX_BAG)
		{
			if (data.pos2 < USER_MAX_BAG)
			{
				bool iscount = false;
				// ����-������1.�Ƿ���Ե���							2.���ɵ��ӣ�ֱ�ӽ������У�
				if (prop1->isSame(prop2) && s_prop1->type == EPT_CONSUME && s_prop2 && s_prop2->type == EPT_CONSUME)
				{
					// �ɵ���
					s32 maxcount = s_prop2->maxcount;
					if (prop2->base.count >= maxcount) return;

					s32 curcount = prop1->base.count + prop2->base.count;
					//1���������ޣ�
					if (curcount > maxcount)
					{
						prop2->base.count = maxcount;
						curcount = curcount - maxcount;
						prop1->base.count = curcount;
					}
					else
					{
						prop2->base.count = curcount;
						prop1->reset();
					}
					iscount = true;
				}
				else
				{
					//���ɵ��ӣ�ֱ�ӽ����ڴ����
					S_ROLE_PROP temp;
					memcpy(&temp, prop1, sizeof(S_ROLE_PROP));
					memcpy(prop1, prop2, sizeof(S_ROLE_PROP));
					memcpy(prop2, &temp, sizeof(S_ROLE_PROP));
				}
				// ����
				TS_Broadcast::db_SendUpdateProp(user, data.pos1, iscount);
				TS_Broadcast::db_SendUpdateProp(user, data.pos2, iscount);
				TS_Broadcast::do_SendUpdateProp(user, data.pos1, iscount);
				TS_Broadcast::do_SendUpdateProp(user, data.pos2, iscount);
			}
		}
		else if (data.pos1 < USER_MAX_BAG + ERP_COUNT)
		{
			if (data.pos2 < USER_MAX_BAG)
			{
				// װ��-������1.������pos2�ж������Ҹ���λ�ý�����		2.û����������ֱ�ӽ���
				if (prop2->IsT())
				{
					prop2 = user->role.stand.bag.findEmpty(data.pos2);
					if (prop2 == nullptr)
					{
						sendErrInfo(ts, c->ID, CMD_890, 3006, &data, sizeof(S_BAGEQUIPSWAP_DATA));
						return;
					}
				}

				S_ROLE_PROP temp;
				memcpy(&temp, prop1, sizeof(S_ROLE_PROP));
				memcpy(prop1, prop2, sizeof(S_ROLE_PROP));
				memcpy(prop2, &temp, sizeof(S_ROLE_PROP));

				TS_Broadcast::db_SaveCompat(user, data.pos2, data.pos1 - USER_MAX_BAG, 1);
				TS_Broadcast::do_SendCompat(user, data.pos2, data.pos1 - USER_MAX_BAG, 1);

				user->updateAtk(true);
				TS_Broadcast::do_SendValue(CMD_710, user->node.index, user->role.base.life.hp, user);
				TS_Broadcast::do_SendValue(CMD_720, user->node.index, user->role.base.life.mp, user);
				TS_Broadcast::do_SendAtk(user, user);
			}
			else if (data.pos2 < USER_MAX_BAG + ERP_COUNT)
			{
				// װ��-װ����1.pos2��pos1��װ��λ��ͬ���ͣ�ֱ�ӽ������ɣ�				2.���߲�ͬ���ͣ�������
				if ((data.pos1 - USER_MAX_BAG == ERP_RING_LEFT && data.pos2 - USER_MAX_BAG == ERP_RING_RIGHT)
					|| (data.pos1 - USER_MAX_BAG == ERP_RING_RIGHT && data.pos2 - USER_MAX_BAG == ERP_RING_LEFT)
					|| (data.pos1 - USER_MAX_BAG == ERP_BANGLE_LEFT && data.pos2 - USER_MAX_BAG == ERP_BANGLE_RIGHT)
					|| (data.pos1 - USER_MAX_BAG == ERP_BANGLE_RIGHT && data.pos2 - USER_MAX_BAG == ERP_BANGLE_LEFT))
				{
					S_ROLE_PROP temp;
					memcpy(&temp, prop1, sizeof(S_ROLE_PROP));
					memcpy(prop1, prop2, sizeof(S_ROLE_PROP));
					memcpy(prop2, &temp, sizeof(S_ROLE_PROP));
					TS_Broadcast::db_SaveCompat(user, data.pos1 - USER_MAX_BAG, data.pos2 - USER_MAX_BAG, 2);
					TS_Broadcast::do_SendCompat(user, data.pos1 - USER_MAX_BAG, data.pos2 - USER_MAX_BAG, 2);
				}			
			}
		}
	}


	bool AppBag::onServerCommand(net::ITCPServer * ts, net::S_CLIENT_BASE * c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppBag close...line:%d \n", __LINE__);
			return false;
		}
		if (c->clientType != func::S_TYPE_GATE)
		{
			LOG_MESSAGE("AppBag close...line:%d \n", __LINE__);
			return false;
		}

		switch (cmd)
		{
			case CMD_820:onGetCompat(ts, c);            break;//��ȡ�������ս��װ������
			case CMD_830:onBag_Get(ts, c);              break;//��ȡ��������
			case CMD_840:onBag_Combine(ts, c);          break;//����������
			case CMD_850:onBag_EquipUp(ts, c);          break;//����װ��
			case CMD_860:onBag_EquipDown(ts, c);        break;//����װ��
			case CMD_870:onBag_Sell(ts, c);				break;//���߳���
			case CMD_890:onBagEquip_Swap(ts, c);		break;// ���ӽ���
			break;
		}
		return true;
	}











}