#include "RobotData.h"
#include "RobotBase.h"
#include "MyCharacterBase.h"
#include "MyGameInstance.h"
#include <vector>
int32 temp_CheckTime;


void updateRobotData()
{
	if (__MyCharacter == NULL || __AppGameInstance == NULL) return;

	int32 value = __AppGameInstance->GetTimeSeconds() - temp_CheckTime;
	if (value < 100) return;
	temp_CheckTime = __AppGameInstance->GetTimeSeconds();

	for (auto it = __robotDatas.CreateConstIterator(); it; ++it)
	{
		FROBOT* robot = (FROBOT*)& it.Value();

		ARobotBase* view = NULL;
		FVector CheckPos;
		if (robot->view == nullptr) continue;

		view = (ARobotBase*)robot->view;
		CheckPos = view->GetActorLocation();

		bool isrect = __MyCharacter->isMainRect(CheckPos);
		int32 ftime = __AppGameInstance->GetTimeSeconds() - robot->temp.temp_HeartTime;
		if (isrect == false || ftime > 30 * 1000)
		{
			destroyRobot(robot);
		}
	}
}



extern FROBOT* robot_Find(int32 index)
{
	FROBOT* data = __robotDatas.Find(index);
	return data;
}

extern void robot_Add(int32 index, FROBOT* robot)
{
	__robotDatas.Add(index, *robot);
}

extern void robot_Remove(int32 index)
{
	__robotDatas.Remove(index);
}

extern ARobotBase* robotView_Find(int32 index)
{
	ARobotBase** viewptr = __robotViews.Find(index);
	if (viewptr == NULL) return nullptr;
	ARobotBase* view = *viewptr;
	return view;
}

extern void robotView_Add(int32 index, ARobotBase* robot)
{
	__robotViews.Add(index, robot);
}

extern void robotView_Remove(int32 index)
{
	__robotViews.Remove(index);
}

extern void clearAllRobot()
{
	__robotDatas.Empty();
	__robotViews.Empty();
}

void destroyRobot(FROBOT* robotdata)
{
	if (!robotdata) return;
	if (robotdata->view)
	{
		ARobotBase* view = (ARobotBase*)robotdata->view;
		//0 销毁显示对象
		if (view != nullptr) view->Destroy();
	}
	
	//1、移除显示对象
	robotView_Remove(robotdata->data.robotindex);
	robot_Remove(robotdata->data.robotindex);
}