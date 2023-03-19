// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_BagGrid.h"
#include "Components/UniformGridPanel.h"
#include "UI_BagGridSlot.h"
#include "UserData.h"
#include "Components/UniformGridSlot.h"
#include "Blueprint/DragDropOperation.h"
#include "AppBag.h"

void UUI_BagGrid::NativeConstruct()
{
	Super::NativeConstruct();
	// LayoutInventorySlot();
	// 先把BagSlotArray初始化好（即弄上150个空指针）
	if (BagSlotArray.Num() == MAX_USER_BAG)
	{
		for (int i = 0; i < MAX_USER_BAG; i++)
		{
			BagSlotArray[i] = nullptr;
		}
		return;
	}
	for (int i = 0; i < MAX_USER_BAG; i++)
	{
		UUI_BagGridSlot* b = nullptr;
		BagSlotArray.Push(b);
	}
}

// 为什么会走这里呢？
// 因为背包格子不一定会把这个背包占满，可能会有空缺
// 当拖拽到空缺上停下时就做下面的处理
bool UUI_BagGrid::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	bool bDrop = false;

	if (UDragDropOperation* DragDropOperation = Cast<UDragDropOperation>(InOperation))
	{
		// Payload : If you were building an inventory screen this would be the UObject representing the item being moved to another slot.
		if (UUI_BagGridSlot* DraggedSlot = Cast<UUI_BagGridSlot>(InOperation->Payload))
		{
			// 拖拽的最终位置在背包上面
			// 1.背包到背包(没有落脚点，不处理)    2.装备到背包
			int32 pos = DraggedSlot->GetSlotPos();
			DraggedSlot->ShowSlot();

			if (pos < 0) return true;

			if (pos >= MAX_USER_BAG && pos < MAX_USER_BAG + MAX_USER_EQUIP && app::__AppBag)
			{
				// 装备到背包，其实就是脱装备
				FROLE_PROP* prop = &__myUserData.stand.combat.equips[pos - MAX_USER_BAG];
				if (prop->IsT())
				{
					app::__AppBag->send_EquipDown(pos - MAX_USER_BAG, prop->base.id);

				}
			}
			bDrop = true;
		}
	}
	return bDrop;
}

void UUI_BagGrid::LayoutInventorySlot()
{
	SlotArrayInventory->ClearChildren();
	int len = __myUserData.stand.bag.num;
	len = 64;
	if (len <= 0 || len > MAX_USER_BAG) return;

	int32 cur_row = 0;
	int32 cur_col = 0;
	if (InventorySlotClass)
	{
		// len = 64;
		for (int i = 0; i < len; i++)
		{
			UUI_BagGridSlot* SlotWidget = CreateWidget<UUI_BagGridSlot>(GetWorld(), InventorySlotClass);
			if (SlotWidget)
			{
				UUniformGridSlot* GridSlot = SlotArrayInventory->AddChildToUniformGrid(SlotWidget);
				if (GridSlot)
				{
					GridSlot->SetRow(cur_row);
					GridSlot->SetColumn(cur_col);
					GridSlot->SetHorizontalAlignment(HAlign_Fill);
					GridSlot->SetVerticalAlignment(VAlign_Fill);
					cur_col += 1;
					if (cur_col > 5)
					{
						cur_col = 0;
						cur_row += 1;
					}
				}
				BagSlotArray[i] = SlotWidget;
				SlotWidget->SetSlotPos(i);
				SlotWidget->SetSlotInfo(__myUserData.stand.bag.bags[i]);
			}
		}
	}
}

// 某个格子全部更新
void UUI_BagGrid::updateSpecifyGrid(uint8 index)
{
	if (index < 0 || index >= BagSlotArray.Num()) return;
	UUI_BagGridSlot* gridslot = BagSlotArray[index];
	// 更新
	if (!gridslot)return;
	gridslot->SetSlotInfo(__myUserData.stand.bag.bags[index]);
}
// 只更新某个格子的数量
void UUI_BagGrid::updateSpecifyGridNum(uint8 index)
{
	if (index < 0 || index >= BagSlotArray.Num()) return;
	UUI_BagGridSlot* gridslot = BagSlotArray[index];
	// 更新
	if (!gridslot)return;
	gridslot->SetSlotItemNum(__myUserData.stand.bag.bags[index].base.count);
}
