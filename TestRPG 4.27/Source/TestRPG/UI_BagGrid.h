// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_BagGrid.generated.h"

class UUniformGridPanel;
class UUI_BagGridSlot;
/**
 * 
 */
UCLASS()
class TESTRPG_API UUI_BagGrid : public UUserWidget
{
 	GENERATED_BODY()
	UPROPERTY(meta = (BindWidget))
		UUniformGridPanel* SlotArrayInventory;
	UPROPERTY(EditDefaultsOnly, Category = "BagUI")					// Create\spawnʱ�������õ����TSubclassOf<UUI_InventorySlot>
		TSubclassOf<UUI_BagGridSlot> InventorySlotClass;		// ����ʹ����ͼ��䣬�����������

	TArray<UUI_BagGridSlot*> BagSlotArray;

public:
	virtual void NativeConstruct();

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);

	void LayoutInventorySlot();
// 		UUI_BagGrid(const FObjectInitializer& ObjectInitializer);
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateSpecifyGrid(uint8 index);
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void updateSpecifyGridNum(uint8 index);
};
