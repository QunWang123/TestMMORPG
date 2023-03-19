#include "UserData.h"
#include "MyCharacterBase.h"
#include "BlueprintFunction.h"
#include "MyGameInstance.h"

#pragma optimize( "", off )
void onEvent_ToSRC(TArray<FString>& arr)
{
	if (arr.Num() != 4) return;
	FVector pos;
	pos.X = FCString::Atoi(*arr[1]);
	pos.Y = FCString::Atoi(*arr[2]);
	pos.Z = FCString::Atoi(*arr[3]);

	if (__MyCharacter == nullptr) return;
	__MyCharacter->onBackPosCommand(3000, pos);
}

int32 onEvent_Skill(TArray<FString>& arr, FOTHER_ROLE_BASE* data)
{
	if (arr.Num() != 8) return -1;

	int32 skillid = FCString::Atoi(*arr[1]);
	int8 skilllevel = FCString::Atoi(*arr[2]);
	int8 lock_type = FCString::Atoi(*arr[3]);
	int8 lock_index = FCString::Atoi(*arr[4]);

	FVector pos;
	pos.X = FCString::Atoi(*arr[5]);
	pos.Y = FCString::Atoi(*arr[6]);
	pos.Z = FCString::Atoi(*arr[7]);

	if (data == NULL)
	{
		// 自己的技能
		FUSER_ROLE_EVENT* events = &__myUserData.events;
		events->isEventing = true;
		events->event_Type = (uint8)E_EVENT_TYPE::EVENT_SKILL;
		events->event_LockAtk.lock_SkillID = skillid;
		events->event_LockAtk.lock_Type = lock_type;
		events->event_LockAtk.lock_Index = lock_index;
		events->event_LockAtk.lock_Targetpos = pos;

		if (lock_type == (uint8)E_NODE_TYPE::N_ROLE)
		{
			ACharacterBase* Enemy = UBlueprintFunction::findUserView(lock_index);
			if (Enemy != nullptr) events->event_LockAtk.lock_Targetpos = Enemy->GetActorLocation();
		}

		if (__AppGameInstance)
		{
			__myUserData.tmp.skillUseTime = __AppGameInstance->GetTimeSeconds();
		}
		// 调用技能播放
		__MyCharacter->onSkillCommand_Play();
		return 0;
	}

	if (data->view == NULL) return -1;
	ACharacterBase* view = (ACharacterBase*)data->view;

	// 技能必须要在目标位置才能放
	float distance = FVector::Dist(view->GetActorLocation(), data->pos);
	if (distance > 10) return -2;

	FUSER_ROLE_EVENT* events = &data->events;
	events->isEventing = true;
	events->event_Type = (uint8)E_EVENT_TYPE::EVENT_SKILL;
	events->event_LockAtk.lock_SkillID = skillid;
	events->event_LockAtk.lock_Type = lock_type;
	events->event_LockAtk.lock_Index = lock_index;
	events->event_LockAtk.lock_Targetpos = pos;

	if (lock_type == (uint8)E_NODE_TYPE::N_ROLE)
	{
		ACharacterBase* Enemy = UBlueprintFunction::findUserView(lock_index);
		if (Enemy != nullptr) events->event_LockAtk.lock_Targetpos = Enemy->GetActorLocation();
	}

	view->onSkillCommand_Play();

	return 0;
}

void onEvent_Dead(FOTHER_ROLE_BASE* data)
{
	if (data == NULL)
	{
		__myUserData.base.life.hp = 0;
		__MyCharacter->onCommand_Dead();
		return;
	}
	data->hp = 0;
	if (data->view == NULL) return;
	ACharacterBase* view = (ACharacterBase*)data->view;
	if (view == NULL) return;
	view->onCommand_Dead();
}

void renderUserEvent(FOTHER_ROLE_BASE* data)
{
	FUSER_ROLE_EVENT* events = nullptr;
	if (data == NULL)
	{
		events = &__myUserData.events;
	}
	else
	{
		events = &data->events;
	}

	if (events->event_List.Num() == 0 || events->isEventing) return;

	FString msg = events->event_List[0];
	// 拆分字符串
	TArray<FString> arr;
	msg.ParseIntoArray(arr, TEXT(","), true);
	if (arr.Num() == 0) return;

	int32 event_type = FCString::Atoi(*arr[0]);
	switch ((E_EVENT_TYPE)event_type)
	{
	case E_EVENT_TYPE::EVENT_TOSRC:		// 拉回原点
		onEvent_ToSRC(arr);
		break;
	case E_EVENT_TYPE::EVENT_SKILL:
		if (onEvent_Skill(arr, data) != 0) return;
		break;
	case E_EVENT_TYPE::EVENT_DEAD:
		onEvent_Dead(data);
		break;
	}

	// 最后移除第一个元素
	events->event_List.RemoveAt(0);
}

void renderUserEvent()
{
	// 渲染自己
	renderUserEvent(NULL);
	// 渲染别人的事件
	for (auto it = onLineDatas.CreateConstIterator(); it; ++it)
	{
		FOTHER_ROLE_BASE* data = (FOTHER_ROLE_BASE*)&it.Value();
		renderUserEvent(data);
	}


}
#pragma optimize( "", on )