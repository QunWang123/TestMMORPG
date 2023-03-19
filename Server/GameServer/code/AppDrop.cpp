
#include  "AppDrop.h"
#include  "WorldMap.h"
#include  "UserManager.h"
#include  "WorldTools.h"
#include  "WorldBC.h"
#include "AppGlobal.h"
namespace app
{
	IContainer*  __AppDrop;
	u32  checkmapid = 0;
	s64  checktime = 0;

	AppDrop::AppDrop(){}
	AppDrop::~AppDrop(){}

	void AppDrop::onUpdate()
	{
		//200������һ��  һ�μ��һ�ŵ�ͼ
		s32 value = global_gametime - checktime;
		if (value < 200) return;
		checktime = global_gametime;
        
		if (checkmapid >= MAX_MAP_ID) checkmapid = 0;
		S_WORLD_MAP* map = nullptr;
		bool isfind = false;
		for (u32 i = checkmapid; i < MAX_MAP_ID; i++)
		{
			map = __World->getMap(i);
			if (map == nullptr) continue;
			if (!map->isUsed)   continue;
			checkmapid = i + 1;
			isfind = true;
			break;
		}
		if (!isfind)
		{
			for (u32 i = 0; i < MAX_MAP_ID; i++)
			{
				map = __World->getMap(i);
				if (map == nullptr) continue;
				if (!map->isUsed)   continue;
				checkmapid = i + 1;
				isfind = true;
				break;
			}
		}
		if (!isfind) return;
		

		S_RECT_BASE* edge = &map->Node.rect;
		for (u32 row = edge->top; row <= edge->bottom; row++)
		for (u32 col = edge->left; col <= edge->right; col++)
		{
			if (!map->IsRect(row, col)) continue;

			S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
			while (node != nullptr)
			{
				if (node->type != N_PROP)
				{
					node = node->downnode;
					continue;
				}
				S_WORLD_DROP_BASE* drop = map->Drop.findDrop(node->index);
				if (drop == nullptr)
				{
					node = node->downnode;
					continue;
				}
				s32 value = global_gametime - drop->lock_time;
				u32 index = node->index;
				s32 layer = node->layer;
				node = node->downnode;
			
				//��������ʱ��
				if (value > 100000 && value < 120000)
				{
					if (drop->lock_index >= 0 || drop->lock_teamid > 0)
					{
						drop->lock_index = -1;
						drop->lock_teamid = -1;

						LOG_MESSAGE("AppDrop 30��ʱ�䵽 ��������Ϊ����:mapid:%d/%d big:%d/%d\n",
							map->mapID,index, drop->bc.grid_big.row, drop->bc.grid_big.col);
					}
				}
				else if (value >= 120000)
				{
					LOG_MESSAGE("AppDrop 120��ʱ�䵽 ����ɾ��:mapid:%d/%d big:%d/%d\n",
						map->mapID, index, drop->bc.grid_big.row, drop->bc.grid_big.col);

					TS_Broadcast::bc_DropDelete(map->mapID, index);
					map->Drop.leaveWorld(map->mapID, index, layer);
				}
			}
		}
	}

	void AppDrop::clearDrop(s32 mapid,s32 layer)
	{
		auto map = __World->getMap(mapid);
		if (map == NULL) return;


		S_RECT_BASE* edge = &map->Node.rect;

		LOG_MESSAGE("clearDrop....mapid:%d layer:%d count:%d edge:%d/%d  %d/%d \n",mapid,layer,map->Drop.curcount,
			edge->top, edge->bottom, edge->left, edge->right);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		for (u32 col = edge->left; col <= edge->right; col++)
		{
			if (!map->IsRect(row, col))  continue;

			S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
			while (node != nullptr)
			{
				if (node->type != N_PROP)
				{
					node = node->downnode;
					continue;
				}
				S_WORLD_DROP_BASE* drop = map->Drop.findDrop(node->index);
				if (drop == nullptr)
				{
					node = node->downnode;
					continue;
				}
				if(drop->node.layer != layer)
				{
					node = node->downnode;
					continue;
				}

				s32 index = node->index;
				node = node->downnode;

				map->Drop.leaveWorld(map->mapID, index, layer);
			}
		}
	}
	bool AppDrop::onServerCommand(net::ITCPServer * ts, net::S_CLIENT_BASE * c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_CONNECT_SECURE))
		{
			LOG_MESSAGE("AppSkill close...line:%d \n", __LINE__);
			return false;
		}
		if (c->clientType != func::S_TYPE_GATE)
		{
			LOG_MESSAGE("AppDrop close...line:%d \n", __LINE__);
			return false;
		}

		switch (cmd)
		{
		case CMD_6000:
			OnGet(ts, c);
			break;
		}
				
		return true;
	}

	void sendErrInfo(s32 connectid, u16 cmd, u16 childcmd, S_DROP_DATA* data)
	{
		__TcpServer->begin(connectid, cmd);
		__TcpServer->sss(connectid, childcmd);
		__TcpServer->sss(connectid, data, sizeof(S_DROP_DATA));
		__TcpServer->end(connectid);
	}


	//11000-2000 ʰȡĳ��������Ʒ
	void AppDrop::OnGet(net::ITCPServer* ts, net::S_CLIENT_BASE* c)
	{
		S_DROP_DATA data;
		ts->read(c->ID, &data, sizeof(S_DROP_DATA));


		//1������ʹ�ü������
		S_USER_BASE* user = __UserManager->findUser(data.userindex);
		if (user == nullptr)
		{
			sendErrInfo(c->ID, CMD_6000, 3001, &data);
			return;
		}

		//2����֤��ͼ
		S_WORLD_MAP*  map = __World->getMap(user->role.base.status.mapid);
		if (map == nullptr)
		{
			sendErrInfo(c->ID, CMD_6000, 3002, &data);
			return;
		}

		//3 ����
		if (data.dropindex >= map->Drop.maxcount)
		{
			sendErrInfo(c->ID, CMD_6000, 3003, &data);
			return;
		}

		//2����֤����
		S_WORLD_DROP_BASE* drop = map->Drop.findDrop_Safe(data.dropindex, user->node.layer);
		if(drop == NULL || !drop->isused)
		{
			sendErrInfo(c->ID, CMD_6000, 3004, &data);
			return;
		}
		
		if (drop->prop.base.id != data.dropid)
		{
			sendErrInfo(c->ID, CMD_6000, 3005, &data);
			return;
		}
		//5��������֤ ����4��
		u32 distance = TS_Tools::getDistance(&user->bc.grid_pos, &drop->bc.grid_pos);
		if (distance > 3)
		{
			sendErrInfo(c->ID, CMD_6000, 3006, &data);
			return;
		}
		// ��������ʰȡ���
		if (drop->lock_index >= 0 && drop->lock_index != user->node.index)
		{
			sendErrInfo(c->ID, CMD_6000, 3007, &data);
			return;
		}

		switch (drop->prop.base.type)
		{
		case EPT_GOLD:
			user->role.base.econ.gold += drop->prop.base.money;
			TS_Broadcast::do_SendGold(user);//���¾���
			TS_Broadcast::db_SaveGold(user);
			break;
		case EPT_EQUIP:
		case EPT_CONSUME:
			{
			    //��Ч�ĵ��� �鿴���ǲ��������ID
 			    bool isvalid = drop->prop.isPropValid();
 				if (isvalid == false)
 				{
 					sendErrInfo(c->ID, CMD_6000, 3008, &data);
 					return;
 				}

				//��������ӵ���
				script::SCRIPT_PROP* s = (script::SCRIPT_PROP*)drop->prop.temp.script_prop;
				s32 maxcount = s->maxcount;
				if (maxcount <= 0)
				{
					sendErrInfo(c->ID, CMD_6000, 3009, &data);
					return;
				}
				//��֤����������
				bool iscount = false;
				s32 bagpos = user->bag_AddProp(&drop->prop, maxcount, iscount);
				if(bagpos == -1)
				{
					sendErrInfo(c->ID, CMD_6000, 3010, &data);
					return;
				}
				//�ش�DB����
				TS_Broadcast::db_SendUpdateProp(user,bagpos,iscount);
				//���ͱ������߸���
				TS_Broadcast::do_SendUpdateProp(user,bagpos,iscount);
			}
			break;
		}
		LOG_MESSAGE("��ʰȡ�˵���:%d \n", data.dropid);
		
		//�㲥ɾ������ �뿪����
		TS_Broadcast::bc_DropDelete(user->role.base.status.mapid, data.dropindex);
		map->Drop.leaveWorld(map->mapID, drop->node.index, drop->node.layer);
			
		// ��������
		// user->mainTask_Update(0);
	}
}