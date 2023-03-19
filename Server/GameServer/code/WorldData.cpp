#include "WorldData.h"
#include "WorldData.h"
#include "UserManager.h"
#include "AppGlobal.h"
#include "WorldMap.h"
#include "WorldTools.h"
#include "RobotData.h"
#include "WorldBC.h"
#include "AppCopy.h"
#include "AppTeam.h"

namespace app
{
	app::S_USER_BASE* S_WORLD_NODE::nodeToUser(const s32 _layer, const u32 _index, bool isself /*= false*/)
	{
		bool issend = false;
		if (isself == false)
			issend = isLayer(E_NODE_TYPE::N_ROLE, _layer, _index);
		else
			issend = isLayer(E_NODE_TYPE::N_ROLE, _layer);

		if (!issend) return nullptr;
		// �ҵ�����ڵ��������
		S_USER_BASE* user = __UserManager->findUser(index);
		if (user == nullptr || !user->mem.isT()) return nullptr;
		// ��ȡ�����ҵ����ط�����������һЩ�ж�
		net::S_CLIENT_BASE* c = __TcpServer->client(user->tmp.server_connectid, user->tmp.server_clientid);
		if (c == nullptr) return nullptr;
		// ��Ϸ�������п��ܻ���ߺ��Զ������� __TcpServer->client��ȡ����Ϸ����������ID�ͱ���
		// ��ʱ���Ҫͬ��������ҵ�server_connectid��ʹ��������Ϸ��������__TcpServer->client�е�IDһ��
		// user->tmp.server_connectid��ʵ���Ǵ��DB�������ϣ���Ϸ��������Ϊ�ͻ����������д洢��ID
		if (user->tmp.server_connectid != c->ID) user->tmp.server_connectid = c->ID;

		return user;
	}
	// ��ʼ���������
	s32 S_USER_BASE::initNewUser()
	{
		s32 kind = initBornPos(USER_BORN_MAPID);
		if (kind != 0) return kind;

		this->role.base.exp.level = 1;
		this->role.base.exp.currexp = 0;
		this->role.base.status.mapid = USER_BORN_MAPID;
		this->role.base.status.c_mapid = USER_BORN_MAPID;
		this->role.base.status.c_pos = this->role.base.status.pos;
		this->role.base.life.hp = 100;
		this->role.base.life.mp = 100;
		//��ʼ������ �� �ֿ�����
		this->role.stand.bag.num = USER_INIT_BAG;
		
		// this->role.stand.warehouse.num = USER_INIT_WAREHOUSE;
		for (u8 i = 0; i < MAX_SKILL_COUNT; i++)
		{
			u32 value = this->role.base.innate.job * 1000 + 1;
			this->role.stand.myskill.skill[i].id = value + i;
			this->role.stand.myskill.skill[i].level = 1;
		}
		atk_ComputeInitNew();
		return 0;
	}
	// ��ʼ������������
	s32 S_USER_BASE::initBornPos(const u32 mapid)
	{
		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr) return -1;

		S_GRID_BASE* grid = map->Pos.getBorn();
		if (grid == NULL)
		{
			LOG_MESSAGE("init BornPos err... grid==NULL %d-%d\n", mapid, (int)this->mem.id);
			return -2;
		}

		TS_Tools::gridToPos(grid, &this->role.base.status.pos, &map->leftpos);
		this->tmp.bChangeMap = true;
		this->bc.grid_pos = grid;
		return 0;
	}

	s32 S_USER_BASE::initReBornPos(const u32 mapid)
	{
		S_WORLD_MAP* map = __World->getMap(mapid);
		if (map == nullptr)
		{
			LOG_MESSAGE("initReBornPos err map==NULL %d %d\n", mapid, (int)this->mem.id);
			return -1;
		}
		S_GRID_BASE* grid = map->Pos.getReborn();
		if (grid == NULL)
		{
			LOG_MESSAGE("initReBornPos err map==NULL %d %d\n", mapid, (int)this->mem.id);
			return -2;
		}
		TS_Tools::gridToPos(grid, &this->role.base.status.pos, &map->leftpos);
		this->role.base.status.mapid = mapid;
		this->bc.grid_pos = grid;
		return 0;
	}

	// ��ҽ�ɫ����ʱ�ᷢ��ָ�������������������㲥���������
	void S_USER_BASE::enterWorld()
	{
		S_WORLD_MAP* map = __World->getMap(this->role.base.status.mapid);
		if (map == NULL)
		{
			LOG_MESSAGE("enterWorld err map==NULL %d\n", this->role.base.status.mapid);
			return;
		}

		S_ROLE_BASE_STATUS* status = &this->role.base.status;
		status->state = GAME_WAIT;
		TS_Tools::posToGrid(&bc.grid_pos, &status->pos, &map->leftpos);
		TS_Tools::posToGridMax(&bc.grid_big, &status->pos, &map->leftpos);
		map->SetNewEdge(&bc.grid_big, &bc.edge);
		map->PushNode(&bc.grid_big, &node);
		bc.isnodetree = true;
		bc.isedgetemp = true;
		bc.edgetemp.reset();
		bc.edgeold = bc.edge;

		LOG_MESSAGE("enterWorld success %d-%lld, %s-%d-%d ... line:%d\n",
			node.index, mem.id, mem.name, bc.grid_pos.row, bc.grid_pos.col, __LINE__);
	}

	void S_USER_BASE::leaveWorld()
	{
		S_WORLD_MAP* map = __World->getMap(this->role.base.status.mapid);
		if (map == nullptr || bc.isnodetree == false) return;

		map->PopNode(&bc.grid_big, &node);
		bc.isnodetree = false;
		bc.isedgetemp = false;

		role.base.status.state = GAME_FREE;
		LOG_MESSAGE("leaveWorld... %lld-%s ... line:%d\n",
			mem.id, mem.name, __LINE__);
	}

	int S_USER_BASE::isMove(S_VECTOR3* pos)
	{
		if (!this->tmp.temp_BuffRun.isMove()) return -1;

		S_WORLD_MAP* map = __World->getMap(this->role.base.status.mapid);
		if (map == nullptr || !map->isUsed || bc.isnodetree == false) return -2;

		S_GRID_BASE grid;
		TS_Tools::posToGrid(&grid, pos, &map->leftpos);

		bool isgrid = map->IsInGrid(&grid);
		if (!isgrid) return -3;

		// ��֤�����Ƿ�������
		u8 value = map->Data.getCollidesValue(grid.row, grid.col);
		if (value == EGTY_STOP)
		{
			// LOG_MESSAGE("you can't move there...row:%d, col:%d \n", grid.row, grid.col);
			return -4;
		}
		return 0;
	}

	void S_USER_BASE::moveWorld(S_VECTOR3* pos, bool& isupdatarect)
	{
		isupdatarect = false;

		S_WORLD_MAP* map = __World->getMap(this->role.base.status.mapid);
		if (map == nullptr) return;

		S_GRID_BASE big;
		TS_Tools::posToGridMax(&big, pos, &map->leftpos);

		if (!map->IsRect(&big))
		{
			LOG_MESSAGE("moveWorld error... %d-%d ... line:%d\n",
				big.row, big.col, __LINE__);
			return;
		}
		// ����ӽ������仯
		if (bc.grid_big.row != big.row || bc.grid_big.col != big.col)
		{
			isupdatarect = true;
			map->PopNode(&bc.grid_big, &node);
			// ���¸�ֵ����Ӳ���¼�ɵľŹ���
			bc.grid_big = big;
			bc.edgeold = bc.edge;
			// ��ӵ��µĸ�������
			map->PushNode(&bc.grid_big, &node);
			// ���¾Ź�����Ϣ
			map->SetNewEdge(&bc.grid_big, &bc.edge);

			// �����ཻ����
			bc.edgetemp.intersectRect(&bc.edgetemp, &bc.edge);
		}
		// �����Լ��ĸ�������
		role.base.status.pos = *pos;
		TS_Tools::posToGrid(&bc.grid_pos, pos, &map->leftpos);
	}

	bool S_USER_BASE::isLive()
	{
		//return (this->role.base.status.state > E_SPRITE_STATE_FREE && this->role.base.status.state < E_SPRITE_STATE_DEAD);
		return this->role.base.life.hp > 0;
	}

	bool S_USER_BASE::isAtkRole(S_USER_BASE* enemy)
	{
		if (enemy->role.base.exp.level < C_NEW_ROLE_PROTECT_LEVEL) return false;
		if (!enemy->isLive()) return false;
		return true;
	}

	void S_USER_BASE::roleState_SetDead()
	{
		this->role.base.life.hp = 0;
		this->role.base.status.state = E_SPRITE_STATE_DEAD;
	}

	void S_USER_BASE::update()
	{
		s32 value = global_gametime - this->tmp.temp_SkillRun.runtime;
		if (value < 100) return;
		this->tmp.temp_SkillRun.runtime = global_gametime;
		skill_running(100);
		TS_Tools::buff_runing(this, N_ROLE, 100);
		robotThink();
	}

	//���� ����ӷ�Χ�ڵĹ���˼����
	void S_USER_BASE::robotThink()
	{
		S_WORLD_MAP* map = __World->getMap(this->role.base.status.mapid);
		if (map == nullptr) return;
		//����rect
		S_RECT_BASE * edge = &this->bc.edge;
		map->IsRect(edge);

		S_ROBOT * robot = nullptr;
		for (u32 row = edge->top; row <= edge->bottom; row++)
		{

			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (!map->IsRect(row, col)) continue;

				S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					switch (node->type)
					{
					case N_PET:
					case N_MONSTER:
					case N_NPC:
						robot = (S_ROBOT*)TS_Tools::findSprite(&this->bc.edge, node->type, node->index, this->node.layer, this->role.base.status.mapid);
						if (robot == nullptr) break;
						robot->event_AI(this->node.index);
						break;
					}

					node = node->downnode;
				}
			}
		}
	}

	//����л���ͼ3����ʽ
	//1 �л���ͼ 2 �л���· 3 ���븱�� ���ᵼ���л���ͼ
	//kind �л���ͼ�׶�01 ��ʼ�л� ��������  2 �ɹ�
	void S_USER_BASE::changeMap(void* d, u32 size, s32 connectid, u16 cmd, u32 kind)
	{
		if (kind == ECK_LEAVE_NO || kind == ECK_LEAVE_YES)
		{
			//�㲥���ھŹ���Χ����� XX�뿪������
			TS_Broadcast::bc_RoleLeaveWorld(this);
			//�����뿪����
			this->leaveWorld();
			this->node.init(N_ROLE, this->tmp.userindex, -1);
			this->tmp.temp_HeartTime = (u32)time(NULL);

			TS_Broadcast::db_SaveStatusInfo(this);
			if (kind == ECK_LEAVE_NO) return;

			//������
			__UserManager->setOnlineCount(false);
			__UserManager->clearUser(this->tmp.userindex);
			__UserManager->pushUser(this);
			return;
		}
		//��ʼ���ڵ�����
		this->node.init(N_ROLE, this->tmp.userindex, -1);
		//�����������
		this->updateAtk(true);
		//���±������߽ű���ս��װ���ű�
		this->updateBagScript();
		this->updateCombatScript();
		// this->updateWareHouseScript();
		__UserManager->setOnlineCount(true);

		//�����ܵ�����
		TS_Broadcast::do_SendAtk(this, this);
		//�����ɿͻ��������� ���ͻ��˵�ͼ������ɺ󡣡���
		//����900ָ������ ���뵽������ ������һ��ڼ��ص�ͼ ���Ѿ�����...
		//����״̬�������
		TS_Broadcast::db_SaveStatusInfo(this);
	}

	void S_USER_BASE::leaveCopy(bool issend)
	{
		//���û�и��� ֱ���˳�
		if (this->tmp.temp_CopyIndex <= 0)
		{
			LOG_MESSAGE("leaveCopy...1 \n");
			return;
		}

		s32 copyindex = this->tmp.temp_CopyIndex;
		//0 ������Ҽ�¼�ĸ���ID
		this->tmp.setCopyIndex(-1);
		this->node.layer = -1;//����Ϊ������ͼ�㼶 -1

		S_COPY_ROOM* room = __AppCopy->findRoom(copyindex);
		if (room == nullptr)
		{
			LOG_MESSAGE("leaveCopy...2 %d\n", copyindex);
			return;
		}
		if (room->state == ECRS_FREE)
		{
			LOG_MESSAGE("leaveCopy...3 \n");
			return;
		}
		//1 ���˸���
		if (room->maptype == EMT_COPY_ONE)
		{
			if (room->memid != this->mem.id)
			{
				LOG_MESSAGE("leaveCopy...4 memid:%lld/%lld \n", room->memid, this->mem.id);
				return;
			}

			AppCopy::clearCopy(room);
			return;
		}
		//2 ������; �뿪���� = �뿪����
		if (room->state == ECRS_GAMING && issend)
		{
			//֪ͨteamserver xxx�뿪����
			TS_Broadcast::team_LeaveCopy(this, copyindex);
			this->tmp.setTeamIndex(-1, -1);//����뿪����
		}

		//3 �����������ĳ�Ա
		//��������������� �п�����ҵĶ������ҵ�mate���ݱ������������������ʼ����
		//�������ﲻ����mateindex
		auto team = __AppTeam->findTeam(room->teamindex);
		if (team == nullptr) return;

		//ֻҪ��һ�����ڸ������� �����ͻ���� 
		bool isclear = true;
		for (u8 i = 0; i < C_TEAM_PLAYERMAX; i++)
		{
			auto mate = team->findMate(i);
			if (mate == nullptr) continue;
			if (mate->isT() == false) continue;
			if (mate->memid == this->mem.id)
			{
				mate->reset();
				continue;
			}

			auto user = __UserManager->findUser(mate->userindex, mate->memid);
			if (user == nullptr) continue;
			if (user->role.base.status.mapid != room->mapid || user->node.layer != room->index) continue;
			if (user->tmp.temp_CopyIndex != room->index) continue;
			isclear = false;
			break;
		}
		//4 �����ɢ = ������ɢ
		s32 num = team->getNum();
		if (num == 0 || isclear)
		{
			//TS_Broadcast::bc_ResetCopy(room,0);
			AppCopy::clearCopy(room);
		}
	}
}


