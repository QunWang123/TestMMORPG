#include "RobotData.h"
#include "WorldBC.h"
#include "WorldTools.h"
#include "WorldMap.h"
#include "AppGlobal.h"
#include "RobotManager.h"

namespace app
{
	void S_ROBOT::Think_StateUpdate()
	{
		this->data.ai.think = AI_FREE;
		if (tmp.script == nullptr) return;

		bool isup = false;

		if (this->data.status.hp < tmp.script->maxhp)
		{
			s32 value = tmp.script->maxhp / 100;
			this->data.status.hp += value;
			if (this->data.status.hp > tmp.script->maxhp) this->data.status.hp = tmp.script->maxhp;
			isup = true;
		}

		// ͨ������Ĺ㲥�������֪��������û�й���
		// 3-5������һ��Ź㲥
		if (isup)
		{
			// �㲥��������
			TS_Broadcast::bc_RobotState(this, CMD_8200);
			return;
		}
		// �㲥����״̬
		TS_Broadcast::bc_RobotState(this, CMD_8300);
	}


	void S_ROBOT::Think_MoveRand()
	{
		this->data.ai.think = AI_FREE;
		if (tmp.script == nullptr) return;
		if (!tmp.temp_buffRun.isMove())    return;

		u32 dir = TS_Tools::Random(8);
		S_GRID_BASE  grid;
		grid.row = this->bc.grid_pos.row + __Dirs[dir].y;
		grid.col = this->bc.grid_pos.col + __Dirs[dir].x;

		//�ƶ�������֮��
		bool isout = this->data.status.move_edge.outEdge(&grid);
		if (isout)
		{
			isout = this->data.status.move_edge.outEdge(&grid);
			LOG_MESSAGE("Think_MoveRand err node:%d inline:%d\n", this->node.index, __LINE__);
			setMoveReturn();
			return;
		}
		auto map = __World->getMap(this->data.status.mapid);
		if (map == NULL)
		{
			LOG_MESSAGE("Think_MoveRand err %d inline:%d\n", this->data.status.mapid, __LINE__);
			setMoveReturn();
			return;
		}
		bool isno = map->IsSpriteCollide_NoSprite(&grid, this->node.layer, false);
		if (!isno) return;

		//�㲥 �ƶ���ĳ�������������
		TS_Broadcast::bc_RobotState(this, CMD_8400, &grid);
		moveWorld(grid.row, grid.col, dir);
	}

	void S_ROBOT::Think_MoveAtk()
	{
		data.ai.think = AI_FREE;

		//����Ŀ���뿪��������
		bool isdistance = isLockDistance(data.ai.base.lock_type, data.ai.base.lock_id);
		if (!isdistance)
		{
			setLock(N_FREE, -1);
			return;
		}

		//�������ƶ�
		Think_Move(1);
	}

	void S_ROBOT::Think_Move(u8 kind)
	{
		if (tmp.script == nullptr)      return;
		if (tmp.script->alert <= 0)     return;
		if (!tmp.temp_buffRun.isMove()) return;

		//1 �����ƶ�����
		u8 move_dir = 0;
		bool isdir = false;
		switch (kind)
		{
		case 1:
		{
			if (!data.ai.isTimeMoveATK())  return;

			S_GRID_BASE grid;
			void* enemy = TS_Tools::findSprite(&this->bc.edge,
				this->data.ai.base.lock_type,
				this->data.ai.base.lock_id,
				this->node.layer,
				this->data.status.mapid,
				&grid);
			if (enemy == NULL) return;
			isdir = TS_Tools::computeDir(&this->bc.grid_pos, &grid, move_dir);
			//	LOG_MSG("�ƶ�����%d [%d/%d] [%d/%d]\n", move_dir, this->bc.grid_pos.row, this->bc.grid_pos.col,grid.row, grid.col);
		}
		break;
		default:
			if (!data.ai.isTimeMoveReturn())  return;
			isdir = TS_Tools::computeDir(&this->bc.grid_pos, &this->data.status.born_pos, move_dir);
			break;
		}
		if (!isdir) return;

		//2�������ǿ��һ��ǿ���
		if (data.ai.base.pathtype == AI_PATH_FREE)
		{
			if (move_dir < 4) data.ai.base.pathtype = AI_PATH_RIGHT;
			else data.ai.base.pathtype = AI_PATH_LEFT;

			data.ai.base.path_coord1 = bc.grid_pos;
			data.ai.base.pathsave = true;


		}

		//LOG_MSG("�ƶ�����%d ���ң�%d\n", move_dir, data.ai.base.pathtype);

		auto map = __World->getMap(this->data.status.mapid);
		if (map == NULL) return;

		s8 maxdir = 0;//�����
		s8 reuse_dir = 0; //�����õķ���
		s8 current_dir = 0;//��ǰ����
		S_GRID_BASE grid;//�µ�����
		S_GRID_BASE reuse_grid; //�����õ�����

		//���ҷ���ken
		if (data.ai.base.pathtype == AI_PATH_RIGHT)
		{
			maxdir = move_dir + MAX_DIR;
			reuse_dir = MAX_DIR;
			for (u32 i = move_dir; i < maxdir; i++)
			{
				current_dir = i % MAX_DIR;
				grid.col = bc.grid_pos.col + __Dirs[current_dir].x;
				grid.row = bc.grid_pos.row + __Dirs[current_dir].y;

				//��� ���λ���� ��û�� ��������
				bool isno = map->IsSpriteCollide_NoSprite(&grid, this->node.layer, false);
				if (!isno) continue;

				//��������Ŀ���ԣ�Ϊ�˷�ֹ�ػ���·
				if (grid == data.ai.base.path_coord1 || grid == data.ai.base.path_coord2)
				{
					reuse_dir = current_dir;
					reuse_grid = grid;
					continue;
				}
				else
				{
					if (data.ai.base.pathsave)
					{
						data.ai.base.path_coord2 = bc.grid_pos;
						data.ai.base.pathsave = false;
					}
					else
					{
						data.ai.base.path_coord1 = bc.grid_pos;
						data.ai.base.pathsave = true;
					}

					//�������ƶ�
					TS_Broadcast::bc_RobotState(this, CMD_8400, &grid);
					moveWorld(grid.row, grid.col, current_dir);
					if (kind == 1 && data.status.move_edge.outEdge(&grid))
					{
						LOG_MESSAGE("Think_Move return-3 :%d inline:%d\n", node.index, __LINE__);
						setMoveReturn();
					}
					return;
				}

			}
			if (reuse_dir < MAX_DIR)
			{
				TS_Broadcast::bc_RobotState(this, CMD_8400, &reuse_grid);
				moveWorld(reuse_grid.row, reuse_grid.col, reuse_dir);
				if (kind == 1 && data.status.move_edge.outEdge(&reuse_grid))
				{
					LOG_MESSAGE("Think_Move return-4 :%d inline:%d\n", node.index, __LINE__);
					setMoveReturn();
					return;
				}

				if (data.ai.base.pathtype == AI_PATH_RIGHT)  data.ai.base.pathtype = AI_PATH_LEFT;
				else if (data.ai.base.pathtype == AI_PATH_LEFT)  data.ai.base.pathtype = AI_PATH_RIGHT;

				data.ai.base.path_coord1 = bc.grid_pos;
				data.ai.base.path_coord2.init();
				data.ai.base.pathsave = true;
				return;
			}
			return;
		}
		//������
		maxdir = move_dir - MAX_DIR;
		reuse_dir = MAX_DIR;
		for (s32 i = move_dir; i > maxdir; i--)
		{
			if (i < 0) current_dir = MAX_DIR + i;
			else current_dir = i;

			grid.col = bc.grid_pos.col + __Dirs[current_dir].x;
			grid.row = bc.grid_pos.row + __Dirs[current_dir].y;
			if (grid.row < 0) grid.row = 0;
			if (grid.col < 0) grid.col = 0;

			bool isno = map->IsSpriteCollide_NoSprite(&grid, this->node.layer, false);
			if (!isno) continue;

			if (grid == data.ai.base.path_coord1 || grid == data.ai.base.path_coord2)
			{
				reuse_dir = current_dir;
				reuse_grid = grid;
				continue;
			}
			else
			{
				if (data.ai.base.pathsave)
				{
					data.ai.base.path_coord2 = bc.grid_pos;
					data.ai.base.pathsave = false;
				}
				else
				{
					data.ai.base.path_coord1 = bc.grid_pos;
					data.ai.base.pathsave = true;
				}


				//�������ƶ�
				TS_Broadcast::bc_RobotState(this, CMD_8400, &grid);
				moveWorld(grid.row, grid.col, current_dir);
				if (kind == 1 && data.status.move_edge.outEdge(&grid))
				{
					LOG_MESSAGE("Think_Move return-5 :%d inline:%d\n", node.index, __LINE__);
					setMoveReturn();
				}
				//else
				//{
				//	LOG_MSG("Think_Move atk3 %d [%d/%d]inline:%d\n", node.index, grid.row, grid.col, __LINE__);
				//}

				//data.ai.base.pathtype = 0;
				return;
			}
		}
		if (reuse_dir < MAX_DIR)
		{

			TS_Broadcast::bc_RobotState(this, CMD_8400, &reuse_grid);
			moveWorld(reuse_grid.row, reuse_grid.col, reuse_dir);
			if (kind == 1 && data.status.move_edge.outEdge(&reuse_grid))
			{
				LOG_MESSAGE("Think_Move return-5 :%d inline:%d\n", node.index, __LINE__);
				setMoveReturn();
				return;
			}

			if (data.ai.base.pathtype == AI_PATH_RIGHT)  data.ai.base.pathtype = AI_PATH_LEFT;
			else if (data.ai.base.pathtype == AI_PATH_LEFT)  data.ai.base.pathtype = AI_PATH_RIGHT;

			data.ai.base.path_coord1 = bc.grid_pos;
			data.ai.base.path_coord2.init();
			data.ai.base.pathsave = true;

			return;
		}
	}

	void S_ROBOT::Think_SetReborn()
	{
		this->data.ai.base.reset();
		script::SCRIPT_MONSTER_BASE* skillscript = this->tmp.script;
		this->tmp.reset();
		this->tmp.script = skillscript;
		leaveWorld();
		this->data.ai.think = AI_RELIVE_DO;
	}


	void S_ROBOT::Think_Reborn()
	{
		if (this->data.status.state != E_SPRITE_STATE_DEAD) return;
		if (this->data.ai.limit.time_relive > global_gametime) return;
		if (this->tmp.script == nullptr) return;

		if (createInWorld(this->data.status.mapid, this->data.status.born_pos.row, this->data.status.born_pos.col));
		{
			this->data.ai.think = AI_FREE;
			this->data.status.hp = this->tmp.script->maxhp;
			this->data.status.mp = this->tmp.script->maxmp;
			__RobotManager->count += 1;
			return;
		}
		this->data.status.state = E_SPRITE_STATE_FREE;
		init(N_FREE, this->node.index);
	}


	void S_ROBOT::Think_MoveReturn()
	{
		if (tmp.script == nullptr) return;
		u32 col = abs(bc.grid_pos.col - data.status.born_pos.col);
		u32 row = abs(bc.grid_pos.row - data.status.born_pos.row);
		if (row < 2 && col < 2)
		{
			data.ai.think = AI_FREE;
			LOG_MESSAGE("Think_MoveReturn ok 1,line:%d\n", __LINE__);
			return;
		}
		// �ƶ�ʱ�䵽��
		if (data.ai.limit.time_movereturn <= global_gametime)
		{
			data.ai.think = AI_FREE;
			LOG_MESSAGE("Think_MoveReturn ok 2,line:%d\n", __LINE__);
			return;
		}
		Think_Move(0);
	}
}


