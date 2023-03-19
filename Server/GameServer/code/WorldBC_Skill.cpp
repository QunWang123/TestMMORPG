#include "WorldBC.h"
#include "WorldMap.h"
#include "AppGlobal.h"
#include "CommandData.h"

namespace app
{
	// 广播玩家出招
	void TS_Broadcast::bc_RoleSkill(S_USER_BASE* user, S_TEMP_SKILL_RUN_BASE* run)
	{
		if (user == nullptr || run == nullptr) return;
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
					// 广播
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}
						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, true);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						LOG_MESSAGE("BC roleskill... %lld-%d\n", user->mem.id, user->node.index);
						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_3100);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->tmp.user_connectid);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->mem.id);
						__TcpServer->sss(otheruser->tmp.server_connectid, user->node.index);	// 使用技能的玩家索引
						__TcpServer->sss(otheruser->tmp.server_connectid, run->skill_id);
						__TcpServer->sss(otheruser->tmp.server_connectid, run->skill_level);
						__TcpServer->sss(otheruser->tmp.server_connectid, run->lock_type);
						__TcpServer->sss(otheruser->tmp.server_connectid, run->lock_index);
						__TcpServer->sss(otheruser->tmp.server_connectid, &run->targetpos, sizeof(S_VECTOR3));
						__TcpServer->end(otheruser->tmp.server_connectid);

						node = node->downnode;
					}
				}
			}
		}
	}


	void TS_Broadcast::bc_RoleSkillHP(S_USER_BASE* user, u8 hp_type, s32 hp_index, s32 atk, s32 curhp)
	{
		if (user == nullptr) return;
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
					// 广播
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}
						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, true);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						LOG_MESSAGE("BC roleskill... %lld-%d\n", user->mem.id, user->node.index);
						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_3200);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->tmp.user_connectid);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->mem.id);
						__TcpServer->sss(otheruser->tmp.server_connectid, user->node.index);	// 使用技能的玩家索引
						__TcpServer->sss(otheruser->tmp.server_connectid, hp_type);		// 锁定类型
						__TcpServer->sss(otheruser->tmp.server_connectid, hp_index);	// 锁定索引	
						__TcpServer->sss(otheruser->tmp.server_connectid, curhp);
						__TcpServer->sss(otheruser->tmp.server_connectid, atk);
						__TcpServer->end(otheruser->tmp.server_connectid);

						node = node->downnode;
					}
				}
			}
		}
	}


	void TS_Broadcast::bc_RoleDead(S_USER_BASE* user)
	{
		if (user == nullptr) return;
		// if (user->role.base.status.state == GAME_FREE) return;
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
					// 广播
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}
						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, true);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						LOG_MESSAGE("BC roleskill... %lld-%d\n", user->mem.id, user->node.index);
						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_4000);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->tmp.user_connectid);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->mem.id);
						__TcpServer->sss(otheruser->tmp.server_connectid, user->node.index);	// 死亡玩家
						__TcpServer->end(otheruser->tmp.server_connectid);

						node = node->downnode;
					}
				}
			}
		}
	}

	void TS_Broadcast::bc_RoleReborn(S_USER_BASE* user, S_VECTOR3* targetpos)
	{
		if (user == nullptr) return;
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
					// 广播
					S_WORLD_NODE* node = map->Node.trees[row][col].rootnode;
					while (node != nullptr)
					{
						if (node->type != N_ROLE)
						{
							node = node->downnode;
							continue;
						}
						S_USER_BASE* otheruser = node->nodeToUser(user->node.layer, user->node.index, true);
						if (otheruser == nullptr ||
							otheruser->role.base.status.mapid != user->role.base.status.mapid)
						{
							node = node->downnode;
							continue;
						}
						LOG_MESSAGE("BC roleskill... %lld-%d\n", user->mem.id, user->node.index);
						__TcpServer->begin(otheruser->tmp.server_connectid, CMD_4200);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->tmp.user_connectid);
						__TcpServer->sss(otheruser->tmp.server_connectid, otheruser->mem.id);
						__TcpServer->sss(otheruser->tmp.server_connectid, user->node.index);	// 复活玩家索引
						__TcpServer->sss(otheruser->tmp.server_connectid, user->role.base.life.hp);
						__TcpServer->sss(otheruser->tmp.server_connectid, targetpos, sizeof(S_VECTOR3));
						__TcpServer->end(otheruser->tmp.server_connectid);

						node = node->downnode;
					}
				}
			}
		}
	}
}