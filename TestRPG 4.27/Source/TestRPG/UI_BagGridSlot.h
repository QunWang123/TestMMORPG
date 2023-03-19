// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserData.h"
#include "Input/Reply.h"
#include "UI_BagGridSlot.generated.h"


class UImage;
class UTextBlock;
class UButton;
class UUI_BagGridSlotInfo;
class UUI_IconDragDrop;
/**
 * 
 */
UCLASS()
class TESTRPG_API UUI_BagGridSlot : public UUserWidget
{
	GENERATED_BODY()
	UPROPERTY(meta = (BindWidget))
		UImage*			SlotIcon;
	UPROPERTY(meta = (BindWidget))
		UTextBlock*		SlotItemID;
	UPROPERTY(meta = (BindWidget))
		UTextBlock*		SlotItemNum;
	UPROPERTY(meta = (BindWidget))
		UButton*		SlotClickButton;

	UPROPERTY(EditDefaultsOnly, Category = UI)
		TSubclassOf<UUI_BagGridSlotInfo> SlotTipClass;
	UPROPERTY(EditDefaultsOnly, Category = UI)
		TSubclassOf<UUI_IconDragDrop> IconDragDropClass;


	UUI_BagGridSlotInfo* InfoWidget;
	FTimerHandle TimerHandle;
	int32		pos;
	int64		ftime;
	// FROLE_PROP* Prop;
	// FReply Reply;

public:
	virtual void NativeConstruct();

	// 鼠标放在上面一会就显示本格子的数据
	void SpawnInfoSlot();

	void SetSlotItemID(int32 ItemID);			// 这个函数没什么用，全当更新图片使的

	// 每个格子都有pos变量，初始为-1，玩家背包中的150个格子的编号为0-149，装备栏中的是150-159
	// 这样就把装备和背包的格子栏进行了区分
	void SetSlotPos(int32 inPos);
	int32 GetSlotPos();
	FROLE_PROP* GetSlotProp();

	// 拖拽时进行图标的隐藏和显示
	void HideSlot();
	void ShowSlot();

	// ClickProp()应该可以进行优化
	// 优化之后甚至button也可以删掉
	UFUNCTION()
		void ClickProp();
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void SetSlotInfo(FROLE_PROP& inProp);
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void SetSlotItemNum(int32 ItemNum);


private:
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	
	// 拖拽事件，拖拽时原地的那个Slot隐藏(由上面那个NativeOnMouseButtonDown触发)
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation);

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent);
};
