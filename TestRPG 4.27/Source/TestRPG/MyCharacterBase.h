// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacterBase.h"
#include "MyCharacterBase.generated.h"

enum class EAttackType : uint8
{
	NONE,
	SIMPLEATTACK,
	SPECIALATTACK
};
class USpringArmComponent;
class URadialForceComponent;
class UCameraComponent;
class UUI_Bag;
/**
 * 
 */
UCLASS()
class TESTRPG_API AMyCharacterBase : public APlayerCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpringArmComponent")
		USpringArmComponent* EMySpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CamecaComponent")
		UCameraComponent* EMyCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character")
		URadialForceComponent* EMyRadialForceComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RotSpeed")
		float RotSpeed;

	FVector Velocity;
	FTimerHandle TimerHandle;

public:
	void MoveForward(float Val);
	void MoveRight(float Val);
	void Turn(float Val);
	void LookUp(float Val);
	void SetSpringArmLength(float Val);

	EAttackType CurrentAttackType = EAttackType::NONE;
	void Attack(TArray<UAnimMontage*> &InputAnimMontages, EAttackType InAttackType);

	UFUNCTION(BlueprintCallable)
		void SimpleAttack();

	UFUNCTION(BlueprintCallable)
		void SaveAttack();

	UFUNCTION(BlueprintCallable)	/*, BlueprintImplementableEvent*/
		void ResetAttack();

	UFUNCTION(BlueprintCallable)
		void SpecialAttack();

public:

	void ResetProjectileActivate();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackAnimation")
		bool IsAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackAnimation")
		int32 AttackNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackAnimation")
		bool SaveAttackNum;

	// 从0-9，表示最大技能数字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackAnimation")
		int32 LocalSkillIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RadialForce")
		int32 GlideStrength = 15.0;

public:
	UFUNCTION(BlueprintCallable, Category = "MyCharacter")
		void SetCharacter(AMyCharacterBase* player);

	// 拉回上一个坐标点
	UFUNCTION(BlueprintImplementableEvent, Category = "MyCharacter")
		bool onBackPosCommand(int32 childcmd, FVector pos);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyCharacter")
		bool onSkillCommand_Err(int32 err);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyCharacter")
		bool onSkillCommand_ColdTime(int32 skillid, float value);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyCharacter")
		bool onCommand_Economy(int32 Gold, int32 Diamonds);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyCharacter")
		bool onCommand_CurrExp(int32 CurrExp);

	UFUNCTION(BlueprintCallable, Category = "MyCharacter")
		bool isMainRect(FVector tarpos);

	UFUNCTION(BlueprintCallable, Category = "MyCharacter")
		void setLockTarget(E_NODE_TYPE lock_type, int32 lock_index, FVector targetpos);

	UFUNCTION(BlueprintCallable, Category = "MyCharacter")
		int32 isUseSkill(int32 skillindex);

	UFUNCTION(BlueprintCallable, Category = "MyCharacter")
		int32 sendSkill(int32 skillindex);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "MyCharacter")
		int32 useSkill_BP();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MyCharacter")
		int32 isFollowAtk(int32 skillindex);
};

extern AMyCharacterBase* __MyCharacter;
