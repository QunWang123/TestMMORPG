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

	// ����������һ�����ʾ�����ӵ�����
	void SpawnInfoSlot();

	void SetSlotItemID(int32 ItemID);			// �������ûʲô�ã�ȫ������ͼƬʹ��

	// ÿ�����Ӷ���pos��������ʼΪ-1����ұ����е�150�����ӵı��Ϊ0-149��װ�����е���150-159
	// �����Ͱ�װ���ͱ����ĸ���������������
	void SetSlotPos(int32 inPos);
	int32 GetSlotPos();
	FROLE_PROP* GetSlotProp();

	// ��קʱ����ͼ������غ���ʾ
	void HideSlot();
	void ShowSlot();

	// ClickProp()Ӧ�ÿ��Խ����Ż�
	// �Ż�֮������buttonҲ����ɾ��
	UFUNCTION()
		void ClickProp();
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void SetSlotInfo(FROLE_PROP& inProp);
	UFUNCTION(BlueprintCallable, Category = "BagUI")
		void SetSlotItemNum(int32 ItemNum);


private:
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	
	// ��ק�¼�����קʱԭ�ص��Ǹ�Slot����(�������Ǹ�NativeOnMouseButtonDown����)
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation);

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent);
};
