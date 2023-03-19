#include "UserData.h"
#include "BlueprintFunction.h"
#include "MyGameInstance.h"
#include "MyCharacterBase.h"
#include "WorldScript.h"
#pragma optimize( "", off )
void renderUserBuff(FOTHER_ROLE_BASE* otheruser)
{
	FUSER_ROLE_BUFF* buff = NULL;
	ACharacterBase* view = NULL;

	if (otheruser == NULL)
	{
		buff = &__myUserData.tmp.buff_run;
		view = __MyCharacter;
	}
	else
	{
		buff = &otheruser->buff_run;
		view = (ACharacterBase*)otheruser->view;
	}

	bool check = false;
	for (uint32 i = 0; i < MAX_BUFF_COUNT; i++)
	{
		FUSER_ROLE_BUFF_BASE* run = &buff->data[i];
		if (run->buff_id <= 0) continue;
		run->runningtime -= 100;
		if (run->runningtime > 0) continue;
		run->runningtime = 0;
		if (view != NULL) view->onBuffCommand_Play(run->buff_id, 0);
		// buff����
		run->reset();
		check = true;
	}
	if (check == false || otheruser != NULL) return;
	// ����ǵ��ӵĲ����ƶ��Ļ�
	// ��һ�������ƶ�ʧЧ�ˣ�����һ��Ҳ����ű���0
	// ����Ҫ�ж�ʣ���buff���滹�ǲ��Ǻ������״̬
	int32 state = 0;
	for (uint32 i = 0; i < MAX_BUFF_COUNT; i++)
	{
		FUSER_ROLE_BUFF_BASE* run = &buff->data[i];
		if (run->buff_id <= 0) continue;
		script::SCRIPT_BUFF_BASE* buff_script = script::fingScript_Buff(run->buff_id);
		if (buff_script == NULL) continue;
		state = state | buff_script->state;
	}
	buff->state = state;
}

void renderUserBuff()
{
	if (UBlueprintFunction::isLogin() == false) return;
	if (__myUserData.tmp.buff_run.data.Num() != MAX_BUFF_COUNT) return;
	if (__AppGameInstance == NULL) return;
	
	int32 ftime = __AppGameInstance->GetTimeSeconds() - __myUserData.tmp.buff_run.checkTime;
	if (ftime < 100) return;
	__myUserData.tmp.buff_run.checkTime = __AppGameInstance->GetTimeSeconds();

	renderUserBuff(NULL);
	for (auto it = onLineDatas.CreateConstIterator(); it; ++it)
	{
		FOTHER_ROLE_BASE* otheruser = (FOTHER_ROLE_BASE*)&it.Value();
		renderUserBuff(otheruser);
	}
}

#pragma optimize( "", on )