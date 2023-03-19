// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserData.h"
#include "UI_CombatGrid.generated.h"


class UUI_BagGridSlot;
class UTextBlock;
/**
 * 
 */
UCLASS()
class TESTRPG_API UUI_CombatGrid : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* WeaponGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* HeadGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* ClothGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* DecorativeGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* ShoeGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* NecklaceGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* LeftRingGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* RightRingGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* LeftBraceletGrid;
	UPROPERTY(meta = (BindWidget))
		UUI_BagGridSlot* RightBraceletGrid;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* NickText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* InfoText;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* p_atkText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* p_defendText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* m_atkText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* m_defendText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* dodgeText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* critText;

	TArray<UUI_BagGridSlot*> CombatSlotArray;

public:
	virtual void NativeConstruct();

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);


	UFUNCTION()
		void SetSlotsPos();
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateHeadInfo(int32 userindex);							// 这个会在蓝图中调用
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateAllCombatInfo(int32 userindex);						// 这个会在蓝图中
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateCombatSpecifyGrid(uint8 index, FROLE_PROP& inProp);	// 更新装备栏中的某个指定位置，这个只会在底层调用
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateCombatAtk(int32 userindex);
};
