#include "BlueprintFunction.h"
#include "WorldScript.h"
#include "AppSkill.h"
#include "AppDrop.h"

int32 UBlueprintFunction::getSkillScript_Distance(int32 id, int32 level)
{
	script::SCRIPT_SKILL_BASE* sk = script::fingScript_Skill(id, level);
	if (sk == NULL)
	{
		return 10000000;
	}
	return sk->distance;
}


void UBlueprintFunction::send_Reborn(int32 kind)
{
	app::AppSkill::send_Reborn(kind);
}


void UBlueprintFunction::send_GetDrop(int32 dropindex, int32 dropid)
{
	app::AppDrop::send_GetDrop(dropindex, dropid);
}
