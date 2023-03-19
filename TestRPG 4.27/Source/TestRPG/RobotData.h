// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "UserData.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RobotData.generated.h"

class ARobotBase;

#pragma pack(push,packing)
#pragma pack(1)

//�����������
USTRUCT(BlueprintType)
struct  FROBOT_BASE
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		int32 robotindex;//�������� �ڷ������ϵ�λ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		int32 id;//����ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		uint8 dir;//���﷽��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		int32 hp;//��ǰ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		FGRID_BASE grid;//��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		FVector pos;//3D����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		FString nick;//���� �ǳ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_BASE")
		int32 maxhp;//�������
};

USTRUCT(BlueprintType)
struct  FROBOT_TEMP
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT_TEMP")
		int32 temp_HeartTime;//�������� �ڷ������ϵ�λ��
};

#pragma pack(pop, packing)

//������ɫ���ݽṹ��
USTRUCT(BlueprintType)
struct FROBOT
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT")
		FROBOT_BASE       data;//��������

	//��ʾ���������
	void* view;
	//�¼���Ϣ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT")
		FUSER_ROLE_EVENT    events;//�¼���Ϣ
	//buff����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT")
		FUSER_ROLE_BUFF     buff_run;//buff����

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROBOT")
		FROBOT_TEMP			temp;//��ʱ����

	inline void reset()
	{
		view = NULL;
		events.reset();
		buff_run.reset();
	}
};

extern TMap<int32, FROBOT>     __robotDatas;//���е�robot����
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
