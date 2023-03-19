// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RobotData.h"
#include "UserData.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RobotBase.generated.h"

UCLASS()
class TESTRPG_API ARobotBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARobotBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	//怪物数据
	// FROBOT* robotdata;
	int32 rindex;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//1 使用技能
	UFUNCTION(BlueprintImplementableEvent, Category = "FlowerEngine")
		bool onSkillCommand_Play(int32 num);
	//2 收到buff
	UFUNCTION(BlueprintImplementableEvent, Category = "FlowerEngine")
		bool onBuffCommand_Play(int32 id, int32 runningtime);
	//3 使用技能 收到伤害值
	UFUNCTION(BlueprintImplementableEvent, Category = "FlowerEngine")
		bool onSkillCommand_HP(int32 curhp, int32 atk);

	//4 移动
	UFUNCTION(BlueprintImplementableEvent, Category = "FlowerEngine")
		bool onCommand_Move(FVector pos, int32 num);
	//5 收到玩家死亡
	UFUNCTION(BlueprintImplementableEvent, Category = "FlowerEngine")
		bool onCommand_Dead();


	//初始化 事件
	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		void initEventing(int32 index);

	//类函数 继承类可以调用 蓝图调用
	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		void setView(int32 robotindex);

	//类函数 继承类可以调用 蓝图调用
	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		void destroyRobot();

	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
	//	bool isEventing();
	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
	//	int32 eventType();

	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
	//	int32 getEventCount();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
		FLOCK_ATK_DATA getLockData();
	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
	//	FUSER_ROLE_EVENT getEventData();

};
