// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_BagGridSlotInfo.generated.h"
class UTextBlock;
struct FROLE_PROP;
/**
 * 
 */
UCLASS()
class TESTRPG_API UUI_BagGridSlotInfo : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
		UTextBlock*		SlotItemID;

	
public:
	void InitInfo(const FROLE_PROP& InData);
};
