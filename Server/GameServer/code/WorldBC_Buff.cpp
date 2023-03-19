#include "WorldBC.h"
#include "AppGlobal.h"
#include "WorldMap.h"

namespace app
{

	void TS_Broadcast::bc_RoleBuff(S_USER_BASE* user, S_TEMP_BUFF_RUN_BASE* run)
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
					// ¹ã²¥
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

						LOG_MESSAGE("BC rolebuff... %lld-%d\n", user->mem.id, user->node.index);
						TS_Broadcast::do_SendBuff(CMD_5000,
							otheruser->tmp.server_connectid,
							otheruser->tmp.user_connectid,
							otheruser->mem.id,
							user->node.index,
							run);
						node = node->downnode;
					}
				}
			}
		}
	}

	void TS_Broadcast::do_SendBuff(u16 cmd, s32 otherServerID, s32 otherUserID, u64 othermemID, u32 nodeindex, S_TEMP_BUFF_RUN_BASE* run)
	{
		__TcpServer->begin(otherServerID, cmd);
		__TcpServer->sss(otherServerID, otherUserID);
		__TcpServer->sss(otherServerID, othermemID);
		__TcpServer->sss(otherServerID, nodeindex);
		__TcpServer->sss(otherServerID, run, sizeof(S_TEMP_BUFF_RUN_BASE));
		__TcpServer->end(otherServerID);
	}
}