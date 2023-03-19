#include "RobotBase.h"
#include "MyCharacterBase.h"
#include "MyGameInstance.h"
#include "BlueprintFunction.h"
#include "WorldTools.h"
#include "WorldScript.h"
//玩家使用技能事件
void onEvent_RobotSkill(TArray<FString> arr, FROBOT* robot)
{
	if (arr.Num() != 5) return;

	int32 skillindex = FCString::Atoi(*arr[1]);
	int8  skilllevel = FCString::Atoi(*arr[2]);
	int8 lock_type = FCString::Atoi(*arr[3]);
	int32 lock_index = FCString::Atoi(*arr[4]);

	if (robot->view == NULL) return;
	ARobotBase * view = (ARobotBase*)robot->view;
	int32 skillid = 0;
	script::SCRIPT_MONSTER_BASE * s = script::findScript_Monster(robot->data.id);
	if (s != NULL && skillindex < s->skill_ids.size())
	{
		skillid = s->skill_ids[skillindex];
	}
	FUSER_ROLE_EVENT * events = &robot->events;

	events->isEventing = true;
	events->event_Type = (uint8)E_EVENT_TYPE::EVENT_SKILL;
	events->event_LockAtk.lock_SkillID = skillid;
	events->event_LockAtk.lock_Type = lock_type;
	events->event_LockAtk.lock_Index = lock_index;

	
	if (lock_type == (uint8)E_NODE_TYPE::N_ROLE)
	{
		//自己的话 设置目标位置
		if (lock_index == __myUserData.userindex)
		{
			events->event_LockAtk.lock_Targetpos = __MyCharacter->GetActorLocation();
		}
		else
		{
			ACharacterBase* enemy = UBlueprintFunction::findUserView(lock_index);
			if (enemy != NULL) events->event_LockAtk.lock_Targetpos = enemy->GetActorLocation();
		}

	}
	else
	{
		FROBOT* enemy = robot_Find(lock_index);
		if (enemy != NULL && enemy->view != NULL)
		{
			ARobotBase* enemy_view = (ARobotBase*)enemy->view;
			events->event_LockAtk.lock_Targetpos = enemy_view->GetActorLocation();
		}
	}

	int32 num = robot->events.event_List.Num();
	//这里就调用技能播放
	view->onSkillCommand_Play(num);




}

//怪物移动
void onEvent_RobotMove(TArray<FString> arr, FROBOT* robot)
{
	if (arr.Num() != 3)      return;
	if (robot->view == NULL) return;
	robot->events.isEventing = true;
	robot->events.event_Type = (uint8)E_EVENT_TYPE::EVENT_MOVE;

	FVector target_pos;
	FGRID_BASE grid;
	grid.row = FCString::Atoi(*arr[1]);
	grid.col = FCString::Atoi(*arr[2]);

	ARobotBase* view = (ARobotBase*)robot->view;
	gridToPos(&grid, &target_pos, &__CurMapInfo.leftpos);

	int32 num = robot->events.event_List.Num();
	//调用移动
	view->onCommand_Move(target_pos, num);


}


//怪物死亡
void onEvent_RobotDead(FROBOT* robot)
{
	//通知蓝图 调用技能
	if (robot->view == NULL) return;

	robot->events.isEventing = true;
	robot->events.event_Type = (uint8)E_EVENT_TYPE::EVENT_DEAD;

	ARobotBase* view = (ARobotBase*)robot->view;


	robot->data.hp = 0;
	view->onCommand_Dead();
}


void renderRobotEvent(FROBOT* robot)
{
	FUSER_ROLE_EVENT* events = &robot->events;

	if (events->event_List.Num() == 0) return;//如果事件长度==0 
	if (events->isEventing) return;//正在处理事件

	FString msg = events->event_List[0];
	//拆分字符串
	TArray<FString> arr;
	msg.ParseIntoArray(arr, TEXT(","), true);
	if (arr.Num() == 0) return;
	int32 event_type = FCString::Atoi(*arr[0]);

	switch ((E_EVENT_TYPE)event_type)
	{
	case E_EVENT_TYPE::EVENT_SKILL://使用技能
		onEvent_RobotSkill(arr, robot);
		break;
	case E_EVENT_TYPE::EVENT_MOVE://移动
		onEvent_RobotMove(arr, robot);
		break;
	case E_EVENT_TYPE::EVENT_DEAD://死亡
		onEvent_RobotDead(robot);
		break;

	}

	//最后移除第一个元素
	events->event_List.RemoveAt(0);
}

void renderRobotEvent()
{
	for (auto it = __robotDatas.CreateConstIterator(); it; ++it)
	{
		FROBOT* robot = (FROBOT*)& it.Value();
		renderRobotEvent(robot);
	}
}