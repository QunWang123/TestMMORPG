// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacterBase.h"
#include "OtherPlayerCharacterBase.generated.h"

/**
 * 
 */
UCLASS()
class TESTRPG_API AOtherPlayerCharacterBase : public APlayerCharacterBase
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
