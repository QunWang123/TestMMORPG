// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UserData.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	PLAYER_NONE,			// 玩家中立，默认状态
	PLAYER_TEAM,			// 玩家队友
	PLAYER_ENEMY,			// 玩家敌人
	MONSTER_SIMPLE,
	MONSTER_BOSS,
};

UCLASS()
class TESTRPG_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditDefaultsOnly, Category = "CharacterType")
		TEnumAsByte<ECharacterType> CharacterType;

	

public:

	//玩家数据
	FOTHER_ROLE_BASE*  userdata;

	
// 	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Speed")
// 		float Speed;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//判断是不是可以移动
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CharacterBase")
		bool isMove(bool ismy);
	UFUNCTION(BlueprintCallable, Category = "CharacterBase")
		void updatePosition();

	//1 使用技能
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onSkillCommand_Play();
	//2 收到buff
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onBuffCommand_Play(int32 id, int32 runningtime);
	//3 使用技能 收到伤害值
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "CharacterBase")
		bool onSkillCommand_HP(int32 curhp, int32 atk);
	//4 收到玩家死亡
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onCommand_Dead();
	//复活
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onRebornCommand_Play(int32 err, FVector pos);

	//升级
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onCommand_Level(int32 level);

	//初始化 事件
	UFUNCTION(BlueprintCallable, Category = "CharacterBase")
		void initEventing(bool ismy);

	//类函数 继承类可以调用 蓝图调用
	UFUNCTION(BlueprintCallable, Category = "CharacterBase")
		void setView(int32 userindex);

 	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
 		FLOCK_ATK_DATA getLockData();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CharacterBase")
		int32 getEventCount();
};
