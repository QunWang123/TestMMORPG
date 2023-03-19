// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_IconDragDrop.generated.h"
class UTexture2D;
class FText;
/**
 * 
 */
UCLASS()
class TESTRPG_API UUI_IconDragDrop : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
		class UImage* ICON;

	UPROPERTY(meta = (BindWidget))
		class UTextBlock* ItemID;

public:
	void DrawICON(UTexture2D* InICON);
	void DrawItemID(FText inText);
};
