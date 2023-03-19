// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_BagGridSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "AppBag.h"
#include "Components/CanvasPanelSlot.h"
#include "UI_BagGridSlotInfo.h"
#include "UI_IconDragDrop.h"
#include "Blueprint/DragDropOperation.h"
#include "Styling/SlateBrush.h"
#include "Blueprint/WidgetBlueprintLibrary.h"


void UUI_BagGridSlot::NativeConstruct()
{
	Super::NativeConstruct();
	SlotClickButton->OnClicked.AddDynamic(this, &UUI_BagGridSlot::ClickProp);
	pos = -1;
	ftime = 0;
	if (__AppGameInstance)
	{
		ftime = __AppGameInstance->GetTimeSeconds();
	}
	InfoWidget = nullptr;
	// Prop = nullptr;
}

void UUI_BagGridSlot::SetSlotInfo(FROLE_PROP& inProp)
{
	// Prop = &inProp;
	// һЩͼƬʲô��Ҳ�����������
	SetSlotItemID(inProp.base.id);
	SetSlotItemNum(inProp.base.count);
}

// �������ûʲô�ã�ȫ������ͼƬʹ��
void UUI_BagGridSlot::SetSlotItemID(int32 ItemID)
{
	if (ItemID <= 0)
	{
		SlotItemID->SetText(FText::FromString(FString("")));
	}
	else
	{
		SlotItemID->SetText(FText::AsNumber(ItemID));
	}
}

void UUI_BagGridSlot::SetSlotItemNum(int32 ItemNum)
{
	if (ItemNum <= 1)
	{
		SlotItemNum->SetText(FText::FromString(FString("")));
	}
	else
	{
		SlotItemNum->SetText(FText::AsNumber(ItemNum));
	}
}

void UUI_BagGridSlot::ClickProp()
{
	if (pos >= 0)
	{
		if (UGameplayStatics::GetTimeSeconds(GetWorld()) * 1000 - ftime < 200)
		{
			// ���Կ�����˫��
			FString ss = FString::Printf(TEXT("can be regard as double clicked-%d"), pos);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, ss);
			// ����ָ��
			if (app::__AppBag)
			{
				if (pos >= MAX_USER_BAG)
				{
					if (pos < MAX_USER_BAG + MAX_USER_EQUIP)
					{
						FROLE_PROP* prop = &__myUserData.stand.combat.equips[pos - MAX_USER_BAG];
						if (prop->IsT())
						{
							app::__AppBag->send_EquipDown(pos - MAX_USER_BAG, prop->base.id);

						}
					}
				}
				else
				{
					FROLE_PROP* prop = &__myUserData.stand.bag.bags[pos];
					if (prop->IsT())
					{
						app::__AppBag->send_EquipUp(pos, prop->base.id);
					}
				}
			}
			return;
		}
		ftime = UGameplayStatics::GetTimeSeconds(GetWorld()) * 1000;
		// ��ק����֮����˵
	}
}

void UUI_BagGridSlot::SetSlotPos(int32 inPos)
{
	pos = inPos;
}


int32 UUI_BagGridSlot::GetSlotPos()
{
	return pos;
}

FROLE_PROP* UUI_BagGridSlot::GetSlotProp()
{
	if (pos < 0) return nullptr;
	FROLE_PROP* Prop = nullptr;
	if (pos < MAX_USER_BAG)
	{
		if (pos < __myUserData.stand.bag.bags.Num())
		{
			Prop = &__myUserData.stand.bag.bags[pos];
		}
	}
	else if (pos < MAX_USER_BAG + MAX_USER_EQUIP)
	{
		if (pos - MAX_USER_BAG < __myUserData.stand.combat.equips.Num())
		{
			Prop = &__myUserData.stand.combat.equips[pos - MAX_USER_BAG];
		}
	}
	return Prop;
}

void UUI_BagGridSlot::HideSlot()
{
	SlotIcon->SetVisibility(ESlateVisibility::Hidden);
	SlotItemID->SetVisibility(ESlateVisibility::Hidden);
	SlotItemNum->SetVisibility(ESlateVisibility::Hidden);
}

void UUI_BagGridSlot::ShowSlot()
{
	SlotIcon->SetVisibility(ESlateVisibility::Visible);
	SlotItemID->SetVisibility(ESlateVisibility::Visible);
	SlotItemNum->SetVisibility(ESlateVisibility::Visible);
}

FReply UUI_BagGridSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	FROLE_PROP* Prop = GetSlotProp();
	if (Prop && Prop->IsT())
	{
		FReply Reply = FReply::Handled();
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton || InMouseEvent.IsTouchEvent())
		{
			TSharedPtr<SWidget> SlateWidgetDrag = GetCachedWidget();
			if (SlateWidgetDrag)
			{
				Reply.DetectDrag(SlateWidgetDrag.ToSharedRef(), EKeys::RightMouseButton);
				return Reply;
			}
		}
	}
	return FReply::Unhandled();
}

// ��ק
void UUI_BagGridSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	FROLE_PROP* Prop = GetSlotProp();
	if (Prop && Prop->IsT() && IconDragDropClass)
	{
		if (UUI_IconDragDrop* ICONDragDrop = CreateWidget<UUI_IconDragDrop>(GetWorld(), IconDragDropClass))
		{
			if (UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>(GetTransientPackage(), UDragDropOperation::StaticClass()))
			{
				DragDropOperation->SetFlags(RF_StrongRefOnFrame);
				// SlotIcon->Brush;
				UTexture2D* Texture = Cast<UTexture2D>(SlotIcon->Brush.GetResourceObject());
				ICONDragDrop->DrawICON(Texture);
				ICONDragDrop->DrawItemID(SlotItemID->GetText());
				DragDropOperation->DefaultDragVisual = ICONDragDrop;
				DragDropOperation->Payload = this;
				OutOperation = DragDropOperation;
				HideSlot();
			}
		}
	}
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

// ���£��������Ҳ�е㽲�����������ֻ���ڱ����������������װ���Ͻ������������Ͻ�������
// ����Ǳ����Ͻ�װ������Ӧ����ui_combat����ʵ��NativeOnDrop
bool UUI_BagGridSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	bool bDrop = false;

	// NativeOnDrop�Ƕ�������קͣ�µ�ʱ�������Ǹ������Ĵ������
	// �����MyInventorySlot������֮ǰ��ק���Ǹ�����
	// ��ֱ�ӵ��õ�GetGameState()�Ǳ���קͣ�µ�λ�õ��Ǹ�����
	if (UDragDropOperation* DragDropOperation = Cast<UDragDropOperation>(InOperation))
	{
		// Payload : If you were building an inventory screen this would be the UObject representing the item being moved to another slot.
		if (UUI_BagGridSlot* DraggedSlot = Cast<UUI_BagGridSlot>(InOperation->Payload))
		{
			int32 DraggedPos = DraggedSlot->GetSlotPos();

			ShowSlot();
			DraggedSlot->ShowSlot();
			// DraggedPos < 0���������ܷ����������֤����
			if (DraggedPos < 0 || !app::__AppBag || pos < 0 || DraggedPos == pos) return true;

			if (DraggedPos < MAX_USER_BAG)
			{
				if (pos < MAX_USER_BAG)
				{
					// �������������û�
					app::__AppBag->send_Swap(DraggedPos, __myUserData.stand.bag.bags[DraggedPos].base.id,
						pos, __myUserData.stand.bag.bags[pos].base.id);
				}
				else if (pos < MAX_USER_BAG + MAX_USER_EQUIP)
				{
					// ������װ�������û�
					// �������д�ɴ���װ����
					app::__AppBag->send_EquipUp(DraggedPos, __myUserData.stand.bag.bags[DraggedPos].base.id);
				}
			}
			else if (DraggedPos < MAX_USER_BAG + MAX_USER_EQUIP)
			{
				if (pos < MAX_USER_BAG)
				{
					// װ�����������û�
					app::__AppBag->send_Swap(DraggedPos,
						__myUserData.stand.combat.equips[DraggedPos - MAX_USER_BAG].base.id,
						pos, __myUserData.stand.bag.bags[pos].base.id);
				}
				else if (pos < MAX_USER_BAG + MAX_USER_EQUIP)
				{
					// װ����װ�����û�
					app::__AppBag->send_Swap(DraggedPos,
						__myUserData.stand.combat.equips[DraggedPos - MAX_USER_BAG].base.id,
						pos, __myUserData.stand.combat.equips[pos - MAX_USER_BAG].base.id);
				}
			}
			bDrop = true;
		}
	}
	return bDrop;
}


// ��ʾ��Ʒ��Ϣ
void UUI_BagGridSlot::SpawnInfoSlot()
{
	FROLE_PROP* Prop = GetSlotProp();

	if (InfoWidget == nullptr && Prop && Prop->IsT())
	{
		InfoWidget = CreateWidget<UUI_BagGridSlotInfo>(GetWorld(), SlotTipClass);
		if (InfoWidget)
		{
			InfoWidget->AddToViewport(1);
			FVector2D tmppos;
			GetWorld()->GetFirstPlayerController()->GetMousePosition(tmppos.X, tmppos.Y);
			// +5.0f��Ϊ�˷�ֹ�ص�
			tmppos.X += 5.0f;
			tmppos.Y += 5.0f;
			InfoWidget->SetPositionInViewport(tmppos);

			InfoWidget->InitInfo(*Prop);
		}
	}
	if (GetWorld()->GetTimerManager().TimerExists(TimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
}


void UUI_BagGridSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		this, &UUI_BagGridSlot::SpawnInfoSlot, 1.0f, false, 0.35);
}


void UUI_BagGridSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (GetWorld()->GetTimerManager().TimerExists(TimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	if (InfoWidget)
	{
		InfoWidget->RemoveFromParent();
		InfoWidget = nullptr;
	}
}

