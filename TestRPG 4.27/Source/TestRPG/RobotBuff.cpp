#include "BlueprintFunction.h"
#include "MyGameInstance.h"
#include "RobotBase.h"





int64 temp_Bufftime = 0;


void renderRobotBuff(FROBOT* robotdata)
{
	if (robotdata == NULL) return;

	FUSER_ROLE_BUFF* buff = &robotdata->buff_run;
	ARobotBase* view = (ARobotBase*)robotdata->view;

	for (int32 i = 0; i < buff->data.Num(); i++)
	{
		FUSER_ROLE_BUFF_BASE* run =  &buff->data[i];
		if(run->buff_id <= 0) continue;
		run->runningtime -= 100;
		if(run->runningtime > 0) continue;

		FString ss = FString::Printf(TEXT("robot buff delete...[%d]\n"), run->buff_id);
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, ss);

		if (view != NULL) view->onBuffCommand_Play(run->buff_id, 0);
		run->reset();
	}
}

void renderRobotBuff()
{
	if (UBlueprintFunction::isLogin() == false || __AppGameInstance == nullptr) return;

	int32 ftime = __AppGameInstance->GetTimeSeconds() - temp_Bufftime;
	if (ftime > 0 && ftime < 100) return;
	temp_Bufftime = __AppGameInstance->GetTimeSeconds();

	for (auto it = __robotDatas.CreateConstIterator(); it; ++it)
	{
		FROBOT* robotdata = (FROBOT*)& it.Value();
		renderRobotBuff(robotdata);
	}
}


