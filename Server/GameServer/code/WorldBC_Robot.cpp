#include "WorldBC.h"
#include "WorldMap.h"
#include "AppGlobal.h"


namespace app
{
	void TS_Broadcast::bc_RobotState(S_ROBOT* _robot, u16 cmd, S_GRID_BASE* grid /*= NULL*/, s32 atk /*= 0*/)
	{
		if (_robot == nullptr)  return;

		S_WORLD_MAP* map = __World->getMap(_robot->data.status.mapid);
		if (map == NULL) return;
		S_RECT_BASE* edge = &_robot->bc.edge;
		map->IsRect(edge);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (!map->IsRect(row, col)) continue;
				// 广播
				S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					if (node->type != N_ROLE)
					{
						node = node->downnode;
						continue;
					}
					// 只找玩家
					S_USER_BASE* user = node->nodeToUser(_robot->node.layer, 9999999);
					if (user == nullptr ||
						!user->bc.isnodetree || 
						user->role.base.status.mapid != _robot->data.status.mapid)
					{
						node = node->downnode;
						continue;
					}

					__TcpServer->begin(user->tmp.server_connectid, cmd);
					__TcpServer->sss(user->tmp.server_connectid, user->tmp.user_connectid);
					__TcpServer->sss(user->tmp.server_connectid, user->mem.id);
					__TcpServer->sss(user->tmp.server_connectid, _robot->node.index);	// 使机器人ID
					switch (cmd)
					{
					case CMD_8100:
						// 怪物返回
						break;
					case CMD_8200:
						// 生命
						__TcpServer->sss(user->tmp.server_connectid, _robot->data.status.hp);
						break;
					case CMD_8300:
						__TcpServer->sss(user->tmp.server_connectid, _robot->data.status.state);
						break;
					case CMD_8400:
						// 移动
						__TcpServer->sss(user->tmp.server_connectid, grid, sizeof(S_GRID_BASE));
						break;
					case CMD_8500:
						// 技能
						__TcpServer->sss(user->tmp.server_connectid, _robot->data.ai.base.skill_index);
						__TcpServer->sss(user->tmp.server_connectid, _robot->data.ai.base.skill_level);
						__TcpServer->sss(user->tmp.server_connectid, _robot->data.ai.base.lock_type);
						__TcpServer->sss(user->tmp.server_connectid, _robot->data.ai.base.lock_id);
						break;
					}
					__TcpServer->end(user->tmp.server_connectid);
					node = node->downnode;
				}
			}
		}
	}

	void TS_Broadcast::bc_RobotBuff(S_ROBOT* _robot, S_TEMP_BUFF_RUN_BASE* run)
	{
		if (_robot == nullptr)  return;

		S_WORLD_MAP* map = __World->getMap(_robot->data.status.mapid);
		if (map == NULL) return;
		S_RECT_BASE* edge = &_robot->bc.edge;
		map->IsRect(edge);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (!map->IsRect(row, col)) continue;
				S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					if (node->type != N_ROLE)
					{
						node = node->downnode;
						continue;
					}
					// 只找玩家
					S_USER_BASE* user = node->nodeToUser(_robot->node.layer, 9999999);
					if (user == nullptr ||
						!user->bc.isnodetree ||
						user->role.base.status.mapid != _robot->data.status.mapid)
					{
						node = node->downnode;
						continue;
					}

					TS_Broadcast::do_SendBuff(
						CMD_8800,
						user->tmp.server_connectid,
						user->tmp.user_connectid,
						user->mem.id,
						_robot->node.index,
						run);
					node = node->downnode;
				}
			}
		}
	}

	void TS_Broadcast::bc_RobotSkillHP(S_ROBOT* _robot, u8 hp_type, s32 hp_index, s32 atk, s32 curhp)
	{
		if (_robot == nullptr) return;
		S_WORLD_MAP * map = __World->getMap(_robot->data.status.mapid);
		if (map == nullptr) return;

		S_RECT_BASE * edge = &_robot->bc.edge;
		map->IsRect(edge);

		for (u32 row = edge->top; row <= edge->bottom; row++)
		{
			for (u32 col = edge->left; col <= edge->right; col++)
			{
				if (!map->IsRect(row, col)) continue;

				S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
				while (node != nullptr)
				{
					if (node->type != N_ROLE)
					{
						node = node->downnode;
						continue;
					}
					//查找玩家的角色对象
					S_USER_BASE* other = node->nodeToUser(_robot->node.layer, _robot->node.index, true);
					if (other == nullptr || other->role.base.status.mapid != _robot->data.status.mapid)
					{
						node = node->downnode;
						continue;
					}
					//LOG_MSG("BC robot skillhp...-%d\n",  robot->node.index);

					__TcpServer->begin(other->tmp.server_connectid, CMD_8700);
					__TcpServer->sss(other->tmp.server_connectid, other->tmp.user_connectid);
					__TcpServer->sss(other->tmp.server_connectid, other->mem.id);
					__TcpServer->sss(other->tmp.server_connectid, _robot->node.index);//使用技能怪物的userindex
					__TcpServer->sss(other->tmp.server_connectid, hp_type);  //锁定类型
					__TcpServer->sss(other->tmp.server_connectid, hp_index); //锁定索引
					__TcpServer->sss(other->tmp.server_connectid, curhp);  //当前生命
					__TcpServer->sss(other->tmp.server_connectid, atk);    //攻击力
					__TcpServer->end(other->tmp.server_connectid);
					node = node->downnode;
				}
			}
		}	
	}
}