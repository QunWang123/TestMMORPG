#include "WorldData.h"
#include "UserManager.h"
#include "WorldMap.h"
#include "WorldTools.h"
#include "RobotData.h"
#include "WorldBC.h"
namespace app
{
	//更新下背包里面的脚本指针数据
	void S_USER_BASE::updateBagScript()
	{
		for (u8 i = 0; i < USER_MAX_BAG; i++)
		{
			S_ROLE_PROP* prop = &this->role.stand.bag.bags[i];
			if (prop->IsT() == false) continue;
			// 走到这说明bags[i]确实有东西
			prop->temp.script_prop = NULL;
			prop->isPropValid();
		}
	}
	// 更新下战斗装备里面的脚本指针数据
	void S_USER_BASE::updateCombatScript()
	{
		for (u8 i = 0; i < USER_MAX_EQUIP; i++)
		{
			S_ROLE_PROP* prop = &this->role.stand.combat.equip[i];
			if (prop->IsT() == false) continue;
			prop->temp.script_prop = NULL;
			prop->isPropValid();
		}
	}
	s32 S_USER_BASE::bag_AddProp(S_ROLE_PROP* prop, u32 maxcount, bool& iscount)
	{
		u32 num = this->role.stand.bag.num;
		if (num > USER_MAX_BAG)
		{
			LOG_MESSAGE("bag_AddProp  error...%d line:%d \n", num, __LINE__);
			return -1;
		}
		//1、不能合并的道具
		if (maxcount <= 1)
		{
			for (u32 i = 0; i < num; i++)
			{
				S_ROLE_PROP* bag_prop = &this->role.stand.bag.bags[i];
				if (bag_prop->IsT()) continue;
				memcpy(bag_prop,  prop, sizeof(S_ROLE_PROP));
				iscount = false;
				return i;
			}
			return -1;
		}
		//2、可以合并的道具
		s32 findcount = 0;
		for (u32 i = 0; i < num; i++)
		{
			S_ROLE_PROP* bag_prop = &this->role.stand.bag.bags[i];
			if (bag_prop->IsT() && bag_prop->isSame(prop))
			{
				// 叠加
				findcount = prop->base.count + bag_prop->base.count;
				if (findcount <= maxcount)
				{
					// 没有超过最大叠加数量的情况
					bag_prop->base.count = findcount;
					iscount = true;
					return i;
				}
			}
		}
		// 可以合并的道具，但数量超过最大数目，所以另起一个
		for (u32 i = 0; i < num; i++)
		{
			S_ROLE_PROP* bag_prop = &this->role.stand.bag.bags[i];
			if (bag_prop->IsT()) continue;
			memcpy(bag_prop,  prop, sizeof(S_ROLE_PROP));
			iscount = false;
			return i;
		}
		return -1;
	}

	S_ROLE_PROP* S_USER_BASE::bag_FindEmpty(u8& index)
	{
		u32 num = this->role.stand.bag.num;
		if (num > USER_MAX_BAG)
		{
			LOG_MESSAGE("bag_FindEmpty  error...%d line:%d \n", num, __LINE__);
			return nullptr;
		}

		for (u32 i = 0; i < num; i++)
		{
			S_ROLE_PROP* prop = &this->role.stand.bag.bags[i];
			if (prop->IsT()) continue;
			index = i;
			return prop;
		}
		return nullptr;
	}
	u32 S_USER_BASE::bag_GetPropCount(const u32 propid)
	{
		u32 num = this->role.stand.bag.num;
		if (num > USER_MAX_BAG) return 0;
		u32 count = 0;
		for (u32 i = 0; i < num; i++)
		{
			S_ROLE_PROP* bag_prop = &this->role.stand.bag.bags[i];
			if (bag_prop->IsT() && bag_prop->base.id == propid) 
			{
				count += bag_prop->base.count;
			}
		}
		return count;
	}
	//删除指定道具和数量
	void S_USER_BASE::bag_DeleteProp(const u32 propid, const u8 count)
	{
		u32 num = this->role.stand.bag.num;
		if (num > USER_MAX_BAG) return;
		s32 findcount = count;
		for (u32 i = 0; i < num; i++)
		{
			S_ROLE_PROP* prop = &this->role.stand.bag.bags[i];
			if (prop->base.id != propid) continue;

			s32 value = prop->base.count - findcount;
			if (value >= 0)
			{
				prop->base.count = value;
				if (prop->base.count <= 0) prop->reset();

				//回传DB保存
				TS_Broadcast::db_SendUpdateProp(this, i, true);
				//更新玩家
				TS_Broadcast::do_SendUpdateProp(this, i, true);
				break;
			}
			findcount -= prop->base.count;
			prop->base.count = 0;
			prop->reset();
			//回传DB保存
			TS_Broadcast::db_SendUpdateProp(this, i, true);
			//更新玩家
			TS_Broadcast::do_SendUpdateProp(this, i, true);
		}
	}
}