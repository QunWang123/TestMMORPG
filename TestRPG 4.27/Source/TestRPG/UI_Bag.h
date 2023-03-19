// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_Bag.generated.h"
class UUI_BagGrid;
class UUI_CombatGrid;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class TESTRPG_API UUI_Bag : public UUserWidget
{
	GENERATED_BODY()
	UPROPERTY(meta = (BindWidget))
		UUI_BagGrid* BagGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_CombatGrid* ComBatGrid;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* GoldsText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* DiamondsText;

public:
	virtual void NativeConstruct();

	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateAllBagInfo();
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateAllCombatInfo();
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateEconomyInfo();
	
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateBagSpecifyGrid(uint8 index);			// 更新背包中的某个指定位置
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateBagSpecifyGridNum(uint8 index);

	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateCombatSpecifyGrid(uint8 index);
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateCombatAtk();
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateCombatHeadInfo();
};
