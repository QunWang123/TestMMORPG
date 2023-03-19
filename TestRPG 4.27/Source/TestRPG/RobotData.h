// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "UserData.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RobotData.generated.h"

class ARobotBase;

#pragma pack(push,packing)
#pragma pack(1)

//怪物基础数据
USTRUCT(BlueprintType)
struct  FROBOT_BASE
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		int32 robotindex;//怪物索引 在服务器上的位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		int32 id;//怪物ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		uint8 dir;//怪物方向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		int32 hp;//当前生命
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		FGRID_BASE grid;//格子坐标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		FVector pos;//3D坐标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		FString nick;//怪物 昵称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		int32 maxhp;//最大生命
};

USTRUCT(BlueprintType)
struct  FROBOT_TEMP
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_TEMP")
		int32 temp_HeartTime;//怪物索引 在服务器上的位置
};

#pragma pack(pop, packing)

//基础角色数据结构体
USTRUCT(BlueprintType)
struct FROBOT
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT")
		FROBOT_BASE       data;//基础数据

	//显示对象的引用
	void* view;
	//事件信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT")
		FUSER_ROLE_EVENT    events;//事件信息
	//buff数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT")
		FUSER_ROLE_BUFF     buff_run;//buff数据

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT")
		FROBOT_TEMP			temp;//临时数据

	inline void reset()
	{
		view = NULL;
		events.reset();
		buff_run.reset();
	}
};

extern TMap<int32, FROBOT>     __robotDatas;//所有的robot数据
extern TMap<int32, ARobotBase*>  __robotViews;

extern void updateRobotData();

extern void renderRobotEvent();
extern void renderRobotBuff();

extern FROBOT* robot_Find(int32 index);
extern void robot_Add(int32 index, FROBOT* robot);
extern void robot_Remove(int32 index);

extern ARobotBase* robotView_Find(int32 index);
extern void robotView_Add(int32 index, ARobotBase* robot);
extern void robotView_Remove(int32 index);

extern void clearAllRobot();
extern void destroyRobot(FROBOT* robot);


























UCLASS()
class TESTRPG_API ARobotData : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARobotData() { PrimaryActorTick.bCanEverTick = false; };

};
