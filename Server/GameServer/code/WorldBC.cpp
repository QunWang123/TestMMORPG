#include "WorldBC.h"
#include "WorldMap.h"
#include "AppGlobal.h"
#include "ShareFunction.h"
#include "AppTeam.h"
#include "UserManager.h"


namespace app
{
	void TS_Broadcast::team_StartCopy(S_CMD_COPY& data, const u16 cmd)
	{
		if (__TcpCenter == nullptr) return;
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE) return;

		__TcpCenter->begin(cmd);
		__TcpCenter->sss(&data, sizeof(S_CMD_COPY));
		__TcpCenter->end();
	}

	//7400 �뿪����
	void TS_Broadcast::team_LeaveCopy(S_USER_BASE* user, s32 copyindex)
	{
		if (__TcpCenter == nullptr) return;
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE) return;

		S_CMD_COPY data;
		data.userindex = user->tmp.userindex;
		data.memid = user->mem.id;
		data.roomindex = copyindex;
		data.teamindex = user->tmp.temp_TeamIndex;
		data.mapid = user->role.base.status.mapid;

		__TcpCenter->begin(CMD_7400);
		__TcpCenter->sss(&data, sizeof(S_CMD_COPY));
		__TcpCenter->end();
	}

	void TS_Broadcast::bc_ResetCopy(S_COPY_ROOM* room, u16 childcmd)
	{
		if (room == nullptr) return;
		auto team = __AppTeam->findTeam(room->teamindex);
		if (team == nullptr) return;

		for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
		{
			auto mate = team->findMate(i);
			if (mate == nullptr) continue;

			auto user = __UserManager->findUser(mate->userindex, mate->memid);
			if (user == nullptr) continue;
			if (user->tmp.temp_CopyIndex != room->index) continue;

			auto c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
			if (c == nullptr) continue;
			S_CMD_COPY data;
			data.userindex = user->tmp.userindex;
			data.memid = user->mem.id;
			data.user_connectid = user->tmp.user_connectid;
			data.roomindex = room->index;
			data.teamindex = team->index;

			__TcpServer->begin(c->ID, CMD_7401);
			__TcpServer->sss(c->ID, childcmd);
			__TcpServer->sss(c->ID, &data, sizeof(S_CMD_COPY));
			__TcpServer->end(c->ID);
		}
	}

	// �Ź���Χ�ڹ㲥����뿪����
	void TS_Broadcast::bc_RoleLeaveWorld(S_USER_BASE* user)
	{
		if (user->role.base.status.state == GAME_FREE) return;
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);

		if (map == NULL) return;
		S_RECT_BASE* edge = &user->bc.edge;
		map->IsRect(edge);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (map->IsRect(row, col))
				{
					// �㲥
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}
						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, false);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						LOG_MESSAGE("BC UserLeaveWorld %lld-%d\n", user->mem.id, user->node.index);
						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_600);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->tmp.user_connectid);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->mem.id);
						__TcpServer->sss(otheruser->tmp.server_connectid, user->node.index);
						__TcpServer->end(otheruser->tmp.server_connectid);
						 
						node = node->downnode;
					}
				}
			}
		}
	}
	// ������3-5�����͵��Ź�����ң�����ҽ�����Ϸʱ���Ϳ������ȡ������ҵ�����
	void TS_Broadcast::bc_RoleState(S_USER_BASE* user)
	{
		s32 tempTime = (s32)time(NULL) - user->tmp.temp_BCTime;
		if (tempTime < 3) return;
		user->tmp.temp_BCTime = (s32)time(NULL);

		S_ROLE_BASE_STATUS* status = &user->role.base.status;
		bool isup = true;
		
		if (user->role.base.life.hp == 0 || user->role.base.life.hp >= 100)
		{
			isup = false;
		}
		// ��ȡ�����ͼ
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return;
		// ��ȡ��ҹ㲥������
		S_RECT_BASE* edge = &user->bc.edge;
		map->IsRect(edge);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (map->IsRect(row, col))
				{
					// �㲥
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						// ��ɫ���Ͳ���N_ROLE�Ͳ��㲥
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}
						// node�м�¼�Ĳ������Լ�����nodeתΪ���
						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, false);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						if (isup)
						{
							do_SendValue(CMD_710, user->node.index, user->role.base.life.hp, otheruser);
						}
						else
						{
							do_SendValue(CMD_730, user->node.index, user->role.base.status.state, otheruser);
						}
						node = node->downnode;
					}
				}
			}
		}
	}
	// 1100 �㲥xx����������ƶ�
	void TS_Broadcast::bc_RoleMove(S_USER_BASE* user, S_MOVE_ROLE* move)
	{
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return;

		S_RECT_BASE* edge = &user->bc.edge;
		map->IsRect(edge);

		// �����Ź���Χ
		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (map->IsRect(row, col))
				{
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}

						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, false);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}

						move->user_connectid = otheruser->tmp.user_connectid;
						move->memid = otheruser->mem.id;

						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_1100);
						__TcpServer->sss(otheruser->tmp.server_connectid, move, sizeof(S_MOVE_ROLE));
						__TcpServer->end(otheruser->tmp.server_connectid);

						node = node->downnode;
					}
				}
			}
		}
	}
	// xx����뿪�����
	void TS_Broadcast::bc_RoleMoveLeaveBig(S_RECT_BASE* old, S_USER_BASE* user)
	{
		S_WORLD_MAP* map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return;

		map->IsRect(old);

		for (u32 row = old->top; row <= old->bottom; row++)
		{
			for (u32 col = old->left; col <= old->right; col++)
			{
				if (map->IsRect(row, col))
				{
					S_GRID_BASE grid;
					grid.col = col;
					grid.row = row;

					// ����ط���edge�Ѿ�����Ϊ�µ�,�����µľŹ������ĳ��grid�ھɵ�edge�����continue
					if (user->bc.edge.inEdge(&grid)) continue;
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}

						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, false);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						LOG_MESSAGE("BC user move leave...%lld-%d", user->mem.id, user->node.index);

						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_600);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->tmp.user_connectid);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->mem.id);
						__TcpServer->sss(otheruser->tmp.server_connectid, user->node.index);
						__TcpServer->end(otheruser->tmp.server_connectid);

						node = node->downnode;
					}
				}
			}
		}
	}


	//�㲥�������
	void TS_Broadcast::bc_RoleLevelUP(S_USER_BASE* user)
	{
		if (user->role.base.status.state == GAME_FREE) return;
		auto map = __World->getMap(user->role.base.status.mapid);
		if (map == NULL) return;

		//��ȡ�Ź�������
		S_RECT_BASE * edge = &user->bc.edge;
		map->IsRect(edge);

		//�����Ź���
		for (u32 row = edge->top; row <= edge->bottom; row++)
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (map->IsRect(row, col) == false) continue;

				//�������������
				S_WORLD_NODE * node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					if (node->type != N_ROLE)
					{
						node = node->downnode;
						continue;
					}
					S_USER_BASE* other = node->nodeToUser(user->node.layer, user->node.index, false);
					if (other == nullptr || other->role.base.status.mapid != user->role.base.status.mapid)
					{
						node = node->downnode;
						continue;
					}
					LOG_MESSAGE("bc level...%lld-%d\n", user->mem.id, user->node.index);

					__TcpServer->begin(other->tmp.server_connectid, CMD_750);
					__TcpServer->sss(other->tmp.server_connectid, other->tmp.user_connectid);
					__TcpServer->sss(other->tmp.server_connectid, other->mem.id);
					__TcpServer->sss(other->tmp.server_connectid, user->node.index);
					__TcpServer->sss(other->tmp.server_connectid, user->role.base.exp.level);
					__TcpServer->end(other->tmp.server_connectid);

					node = node->downnode;
				}

			}
	}



	void TS_Broadcast::do_SendAtk(S_USER_BASE* user_connect, S_USER_BASE* user)
	{
		__TcpServer->begin(user_connect->tmp.server_connectid, CMD_770);
		__TcpServer->sss(user_connect->tmp.server_connectid, user_connect->tmp.user_connectid);
		__TcpServer->sss(user_connect->tmp.server_connectid, user_connect->mem.id);
		__TcpServer->sss(user_connect->tmp.server_connectid, user->node.index);
		__TcpServer->sss(user_connect->tmp.server_connectid, &user->atk.total, sizeof(S_SPRITE_ATTACK));
		__TcpServer->end(user_connect->tmp.server_connectid);
	}

	// �������ݣ�ʹuser�յ�ĳλ��ҵ�״̬����������	
	void TS_Broadcast::do_SendValue(u16 cmd, u32 index, s32 value, S_USER_BASE* user)
	{
		// v�Լ��Ƿ��͸�������ҵģ�
		S_UPDATE_VALUE v;
		// value�����͸�������ҵ��Լ�������
		v.value = value;
		// targetindex���͸�������ҵ��Լ�����Ϣ
		v.targetindex = index;
		// memid��������ʱ��֤���Ͷ���ģ���ȻҪ��������ҵ�i=memid
		v.memid = user->mem.id;
		// user_connectid����������������ϵ�id
		v.user_connectid = user->tmp.user_connectid;
		// user->tmp.server_connectid����ȻҪ���͸�����������ڵ����ط�����
		__TcpServer->begin(user->tmp.server_connectid, cmd);
		__TcpServer->sss(user->tmp.server_connectid, &v, sizeof(S_UPDATE_VALUE));
		__TcpServer->end(user->tmp.server_connectid);
	}

	void TS_Broadcast::do_SendExp(S_USER_BASE* user)
	{
		__TcpServer->begin(user->tmp.server_connectid, CMD_740);
		__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
		__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
		__TcpServer->sss(user->tmp.server_connectid, user->role.base.exp.currexp);
		__TcpServer->end(user->tmp.server_connectid);
	}

	void TS_Broadcast::do_SendGold(S_USER_BASE* user)
	{
		__TcpServer->begin(user->tmp.server_connectid, CMD_760);
		__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
		__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
		__TcpServer->sss(user->tmp.server_connectid, user->role.base.econ.gold);
		__TcpServer->sss(user->tmp.server_connectid, user->role.base.econ.diamonds);
		__TcpServer->end(user->tmp.server_connectid);
	}

	void TS_Broadcast::do_SendUpdateProp(S_USER_BASE* user, u8 bagpos, bool iscount)
	{
		if (bagpos >= USER_MAX_BAG) return;
		S_ROLE_PROP* prop = &user->role.stand.bag.bags[bagpos];

		__TcpServer->begin(user->tmp.server_connectid, CMD_800);
		__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
		__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
		__TcpServer->sss(user->tmp.server_connectid, bagpos); //����λ��
		__TcpServer->sss(user->tmp.server_connectid, iscount);//������������ȫ�����ݣ�
		//ֻ��������
		if (iscount)
		{
			__TcpServer->sss(user->tmp.server_connectid, prop->base.count); //0 ����ɾ��
		}
		else
		{
			__TcpServer->sss(user->tmp.server_connectid, prop, prop->sendSize());
		}
		__TcpServer->end(user->tmp.server_connectid);
	}

	void TS_Broadcast::do_SendCompat(S_USER_BASE* user, u8 bagpos, u8 equippos, u8 kind)
	{
		auto c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
		if (c == nullptr) return;

		__TcpServer->begin(c->ID, CMD_810);
		__TcpServer->sss(c->ID, user->tmp.user_connectid);
		__TcpServer->sss(c->ID, user->mem.id);
		__TcpServer->sss(c->ID, kind);
		__TcpServer->sss(c->ID, bagpos);
		__TcpServer->sss(c->ID, equippos);
		__TcpServer->end(c->ID);
	}

	void TS_Broadcast::do_SendSwap(S_USER_BASE* user, u8 equippos1, u8 equippos2)
	{
		auto c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
		if (c == nullptr) return;
		__TcpServer->begin(c->ID, CMD_890);
		__TcpServer->sss(c->ID, (u16)0);
		__TcpServer->sss(c->ID, user->tmp.user_connectid);
		__TcpServer->sss(c->ID, user->mem.id);
		__TcpServer->sss(c->ID, equippos1);
		__TcpServer->sss(c->ID, equippos2);
		__TcpServer->end(c->ID);
	}

	//�����л���ͼ�ɹ���Ϣ
	void TS_Broadcast::do_ChangeMap(S_USER_BASE* user, void* d, u32 size, s32 connectid, u16 cmd)
	{
		//�����л���ͼ�ɹ�
		__TcpServer->begin(connectid, cmd);
		__TcpServer->sss(connectid, (u16)0);
		__TcpServer->sss(connectid, d, size);
		__TcpServer->sss(connectid, &user->role.base.status.pos, 12);
		__TcpServer->end(connectid);
	}
	//901 ֪ͨ��ӷ����� ����л���ͼ
	void TS_Broadcast::team_ChangeMap(S_USER_BASE* user, const u16 cmd)
	{
		if (__TcpCenter == nullptr) return;
		if (__TcpCenter->getData()->state < func::C_CONNECT_SECURE) return;

		S_CHANGEMAP_BASE data;
		data.userindex = user->tmp.userindex;
		data.memid = user->mem.id;
		data.user_connectid = user->tmp.user_connectid;
		data.mapid = user->role.base.status.mapid;
		data.line == share::__ServerLine;

		__TcpCenter->begin(cmd);
		__TcpCenter->sss(&data, sizeof(S_CHANGEMAP_BASE));
		__TcpCenter->end();
	}
}