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
	// �Ȱ�BagSlotArray��ʼ���ã���Ū��150����ָ�룩
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

// Ϊʲô���������أ�
// ��Ϊ�������Ӳ�һ������������ռ�������ܻ��п�ȱ
// ����ק����ȱ��ͣ��ʱ��������Ĵ���
bool UUI_BagGrid::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	bool bDrop = false;

	if (UDragDropOperation* DragDropOperation = Cast<UDragDropOperation>(InOperation))
	{
		// Payload : If you were building an inventory screen this would be the UObject representing the item being moved to another slot.
		if (UUI_BagGridSlot* DraggedSlot = Cast<UUI_BagGridSlot>(InOperation->Payload))
		{
			// ��ק������λ���ڱ�������
			// 1.����������(û����ŵ㣬������)    2.װ��������
			int32 pos = DraggedSlot->GetSlotPos();
			DraggedSlot->ShowSlot();

			if (pos < 0) return true;

			if (pos >= MAX_USER_BAG && pos < MAX_USER_BAG + MAX_USER_EQUIP && app::__AppBag)
			{
				// װ������������ʵ������װ��
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

// ĳ������ȫ������
void UUI_BagGrid::updateSpecifyGrid(uint8 index)
{
	if (index < 0 || index >= BagSlotArray.Num()) return;
	UUI_BagGridSlot* gridslot = BagSlotArray[index];
	// ����
	if (!gridslot)return;
	gridslot->SetSlotInfo(__myUserData.stand.bag.bags[index]);
}
// ֻ����ĳ�����ӵ�����
void UUI_BagGrid::updateSpecifyGridNum(uint8 index)
{
	if (index < 0 || index >= BagSlotArray.Num()) return;
	UUI_BagGridSlot* gridslot = BagSlotArray[index];
	// ����
	if (!gridslot)return;
	gridslot->SetSlotItemNum(__myUserData.stand.bag.bags[index].base.count);
}
