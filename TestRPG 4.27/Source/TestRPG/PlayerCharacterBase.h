// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Animation/AnimMontage.h"
#include "PlayerCharacterBase.generated.h"

/**
 * 
 */
UCLASS()
class TESTRPG_API APlayerCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	APlayerCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackAnimation")
		TArray<UAnimMontage*> MovingAttackAnimMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackAnimation")
		UAnimMontage* FallingAttackAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackAnimation")
		TArray<UAnimMontage*> MovingSpeicalAnimMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackAnimation")
		UAnimMontage* FallingSpeicalAnimMontage;

	UFUNCTION()
		void EndSkillMontagePlay(UAnimMontage* InMontage, bool bInterrupted);

	float PlayerPlaySkillAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	FOnMontageEnded OnMontageEnded;

// 
// public:
// 	// Called every frame
// 	virtual void Tick(float DeltaTime) override;
	
};
