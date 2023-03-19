// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

UCLASS()
class TESTRPG_API AMyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		int32   ID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		int32   Row;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlowerEngine")
		int32   Col;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
