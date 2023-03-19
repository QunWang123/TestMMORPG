#include "WorldData.h"
#include "WorldScript.h"
#include "WorldBC.h"


namespace app
{

	void S_USER_BASE::atk_ComputeEquip()
	{
		this->atk.equip.reset();
		for (int i = 0; i < USER_MAX_EQUIP; i++)
		{
			this->atk.equip.hp += this->role.stand.combat.equip[i].equip.hp;
			this->atk.equip.mp += this->role.stand.combat.equip[i].equip.mp;
			this->atk.equip.p_atk += this->role.stand.combat.equip[i].equip.p_atk;
			this->atk.equip.p_defend += this->role.stand.combat.equip[i].equip.p_defend;
			this->atk.equip.m_atk += this->role.stand.combat.equip[i].equip.m_atk;
			this->atk.equip.m_defend += this->role.stand.combat.equip[i].equip.m_defend;
			this->atk.equip.dodge += this->role.stand.combat.equip[i].equip.dodge;
			this->atk.equip.crit += this->role.stand.combat.equip[i].equip.crit;
		}
	}

	void S_USER_BASE::atk_ComputeTotal()
	{
		script::SCRIPT_PALYER_LEVEL* level = script::findScript_Level(this->role.base.exp.level);
		if (level == nullptr)
		{
			LOG_MESSAGE("atk_ComputeTotal err...%d \n", this->role.base.exp.level);
			return;
		}

		this->atk.total.reset();

		this->atk.total.p_atk = this->role.stand.atk.p_atk +
			this->atk.equip.p_atk +
			level->p_atk;
		this->atk.total.p_defend = this->role.stand.atk.p_defend +
			this->atk.equip.p_defend +
			level->p_defend;
		this->atk.total.m_atk = this->role.stand.atk.m_atk +
			this->atk.equip.m_atk +
			level->m_atk;
		this->atk.total.m_defend = this->role.stand.atk.m_defend +
			this->atk.equip.m_defend +
			level->m_defend;
		this->atk.total.dodge = this->role.stand.atk.dodge +
			this->atk.equip.dodge +
			level->dodge;
		this->atk.total.crit = this->role.stand.atk.crit +
			this->atk.equip.crit +
			level->crit;
		this->atk.total.speed = this->role.stand.atk.speed +
			this->atk.equip.speed +
			level->speed;
		this->atk.total.dechp = this->role.stand.atk.dechp +
			this->atk.equip.dechp +
			level->dechp;

		this->atk.total.hp = this->role.stand.atk.hp +
			this->atk.equip.hp +
			level->hp;
		this->atk.total.mp = this->role.stand.atk.mp +
			this->atk.equip.mp +
			level->mp;
		if (this->atk.total.hp < 1)this->atk.total.hp = 1;
		if (this->atk.total.mp < 1)this->atk.total.mp = 1;
	}

	void S_USER_BASE::atk_ComputeInitNew()
	{
		this->role.stand.atk.hp = 100;
		this->role.stand.atk.mp = 100;
		this->role.stand.atk.p_atk = 10;//物理攻击
		this->role.stand.atk.m_atk = 10;//魔法攻击
		this->role.stand.atk.p_defend = 10;//物理防御
		this->role.stand.atk.m_defend = 10;//魔法防御
		this->role.stand.atk.dodge = 10;//闪避
		this->role.stand.atk.crit = 10; //暴击
		this->role.stand.atk.dechp = 10;//伤害减免
		this->role.stand.atk.speed = 10;//移动速度
	}					

	void S_USER_BASE::updateAtk(bool isUpdateHpMp)
	{
		atk_ComputeEquip();
		atk_ComputeTotal();
		if (isUpdateHpMp)
		{
			this->role.base.life.hp = this->atk.total.hp;
			this->role.base.life.mp = this->atk.total.mp;
		}
	}

	void S_USER_BASE::atk_UpdateExp(const s32 exp)
	{
		script::SCRIPT_PALYER_LEVEL* level = script::findScript_Level(this->role.base.exp.level);
		if (level == nullptr)
		{
			LOG_MESSAGE("atk_UpdateExp err...%d \n", this->role.base.exp.level);
			return;
		}

		this->role.base.exp.currexp += exp;
		if (this->role.base.exp.currexp < level->exp)
		{
			//广播玩家经验
			TS_Broadcast::do_SendExp(this);
			TS_Broadcast::db_SendExp(this);
			return;
		}
		// 经验足够升级了
		if (this->role.base.exp.level >= MAX_ROLE_LEVEL)
		{
			if (this->role.base.exp.currexp > level->exp) this->role.base.exp.currexp = level->exp;
			return;
		}
		//验证还能继续升级吗？
		for (u32 i = 0; i < MAX_ROLE_LEVEL; i++)
		{
			this->role.base.exp.level += 1;
			this->role.base.exp.currexp -= level->exp;

			level = script::findScript_Level(this->role.base.exp.level);
			if (level == nullptr)
			{
				LOG_MESSAGE("atk_ComputeLevelup2 err...%d \n", this->role.base.exp.level);
				break;
			}
			if (this->role.base.exp.level >= MAX_ROLE_LEVEL || this->role.base.exp.currexp < level->exp)
			{
				if (this->role.base.exp.currexp > level->exp) this->role.base.exp.currexp = level->exp;
				break;
			}
		}
		// 升级+1000金币
		this->role.base.econ.gold += 1000;
		//更新升级
		updateAtk(true);
		//设置任务
		// mainTask_Update(0);
		//广播 玩家升级
		TS_Broadcast::do_SendGold(this);

		TS_Broadcast::do_SendValue(CMD_710, this->node.index, this->role.base.life.hp, this);
		TS_Broadcast::do_SendValue(CMD_720, this->node.index, this->role.base.life.mp, this);
		TS_Broadcast::do_SendAtk(this, this);
		TS_Broadcast::do_SendExp(this);
		TS_Broadcast::bc_RoleLevelUP(this);
		//回传DB 保存数据
		TS_Broadcast::db_SaveLevelUP(this);
		TS_Broadcast::db_SaveGold(this);
	}
}