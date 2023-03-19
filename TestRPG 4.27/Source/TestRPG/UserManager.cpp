#include "UserData.h"
#include "MyCharacterBase.h"
#include "MyGameInstance.h"
#include "BlueprintFunction.h"
#include "WorldScript.h"

void updateSkillColdTime()
{
	if (__MyCharacter == NULL || 
		__AppGameInstance == NULL || 
		UBlueprintFunction::isLogin() == false ||
		__myUserData.stand.myskill.skill.Num() != MAX_SKILL_COUNT ||
		__myUserData.tmp.skillColdTime.Num() != MAX_SKILL_COUNT || 
		__myUserData.tmp.skillPublicTime <= 0 ||
		__myUserData.tmp.skillUseTime <= 0) return;

	bool isCheck = false;

	for (int i = 0; i < MAX_SKILL_COUNT; i++)
	{
		FROLE_STAND_SKILL_BASE* skill = &__myUserData.stand.myskill.skill[i];
		if (skill == NULL || skill->id <= 0 || skill->level <= 0) continue;

		script::SCRIPT_SKILL_BASE* skillscript = script::fingScript_Skill(skill->id, skill->level);
		if (skillscript == NULL || skillscript->coldtime <= 0) continue;

		// 获取冷却技能的当前时间
		float temp_time = __AppGameInstance->GetTimeSeconds() - __myUserData.tmp.skillColdTime[i];
		float value = 1 - temp_time / (float)skillscript->coldtime;
		if (value >= -0.1f && value <= 1 && __myUserData.tmp.skillColdTime[i] > 0)
		{
			isCheck = true;
			__MyCharacter->onSkillCommand_ColdTime(skill->id, value);
			continue;
		}
		// 走到这说明技能不在冷却中
		temp_time = __AppGameInstance->GetTimeSeconds() - __myUserData.tmp.skillPublicTime;
		value = 1 - temp_time / 300.f;
		if (value >= -0.1f && value <= 1)
		{
			isCheck = true;
			__MyCharacter->onSkillCommand_ColdTime(skill->id, value);
			continue;
		}
	}
	int32 check_time = __AppGameInstance->GetTimeSeconds() - __myUserData.tmp.skillUseTime;
	if (check_time <= 15 * 1000) return;
	if (isCheck == false)__myUserData.tmp.skillUseTime = 0;
}

extern void clearAllUser()
{
	onLineDatas.Empty();
}

