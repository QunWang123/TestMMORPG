// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_CombatGrid.h"
#include "BlueprintFunction.h"
#include "UserData.h"
#include "Components/TextBlock.h"
#include "UI_BagGridSlot.h"
#include "UI_Bag.h"
#include "Blueprint/DragDropOperation.h"
#include "AppBag.h"

void UUI_CombatGrid::NativeConstruct()
{
	Super::NativeConstruct();

	CombatSlotArray.Push(WeaponGrid);
	CombatSlotArray.Push(HeadGrid);
	CombatSlotArray.Push(ClothGrid);
	CombatSlotArray.Push(DecorativeGrid);
	CombatSlotArray.Push(ShoeGrid);
	CombatSlotArray.Push(NecklaceGrid);
	CombatSlotArray.Push(LeftRingGrid);
	CombatSlotArray.Push(RightRingGrid);
	CombatSlotArray.Push(LeftBraceletGrid);
	CombatSlotArray.Push(RightBraceletGrid);
}

bool UUI_CombatGrid::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	bool bDrop = false;

	if (UDragDropOperation* DragDropOperation = Cast<UDragDropOperation>(InOperation))
	{
		// Payload : If you were building an inventory screen this would be the UObject representing the item being moved to another slot.
		if (UUI_BagGridSlot* DraggedSlot = Cast<UUI_BagGridSlot>(InOperation->Payload))
		{
			int32 pos = DraggedSlot->GetSlotPos();
			DraggedSlot->ShowSlot();
			if (pos < 0) return true;

			if (pos < MAX_USER_BAG && app::__AppBag)
			{
				// 背包到装备，穿戴装备
				FROLE_PROP* prop = &__myUserData.stand.bag.bags[pos];
				if (prop->IsT())
				{
					app::__AppBag->send_EquipUp(pos, prop->base.id);
				}
			}
			bDrop = true;
		}
	}
	return bDrop;
}

void UUI_CombatGrid::SetSlotsPos()
{
	for (int i = 0; i < CombatSlotArray.Num(); i++)
	{
		CombatSlotArray[i]->SetSlotPos(i + MAX_USER_BAG);
	}
}

void UUI_CombatGrid::updateHeadInfo(int32 userindex)
{
	if (userindex == __myUserData.userindex)
	{
		FUSER_MEMBER_ROLE role = UBlueprintFunction::getRoleDataBase(__myUserData.select_roleindex);
		if (role.id <= 0)
		{
			return;
		}

		if (NickText)
		{
			NickText->SetText(FText::FromString(role.nick));
		}
		FString Info = FString("level:");
		Info += FString::FromInt(role.level);
		Info += FString(" ");
		Info += UBlueprintFunction::GetJob(role.job);

		if (InfoText)
		{
			InfoText->SetText(FText::FromString(Info));
		}
		return;
	}
	FOTHER_ROLE_BASE* data = onLineDatas.Find(userindex);
	if (data)
	{
		if (data->roleid <= 0)
		{
			return;
		}

		if (NickText)
		{
			NickText->SetText(FText::FromString(data->nick));
		}
		FString Info = FString("level:");
		Info += FString::FromInt(data->level);
		Info += FString(" ");
		Info += UBlueprintFunction::GetJob(data->job);

		if (InfoText)
		{
			InfoText->SetText(FText::FromString(Info));
		}
	}
}

void UUI_CombatGrid::updateAllCombatInfo(int32 userindex)
{
	FROLE_STAND_COMBAT* combat = nullptr;
	if (userindex == __myUserData.userindex)
	{
		combat = &__myUserData.stand.combat;
	}
	else
	{
		FOTHER_ROLE_BASE* data = onLineDatas.Find(userindex);
		if (data)
		{
			combat = &data->combat;
		}
	}
	if (combat)
	{
		for (int8 i = 0; i < CombatSlotArray.Num(); i++)
		{
			CombatSlotArray[i]->SetSlotInfo(combat->equips[i]);
		}
	}
}

void UUI_CombatGrid::updateCombatSpecifyGrid(uint8 index, FROLE_PROP& inProp)
{
	if (index < 0 || index >= CombatSlotArray.Num()) return;
	if (CombatSlotArray[index])
	{
		CombatSlotArray[index]->SetSlotInfo(inProp);
	}
}

void UUI_CombatGrid::updateCombatAtk(int32 userindex)
{
	FSPRITE_ATTACK* atkData = nullptr;
	if (userindex == __myUserData.userindex)
	{
		atkData = &__myUserData.atk;
	}
	else
	{
		FOTHER_ROLE_BASE* data = onLineDatas.Find(userindex);
		if (data)
		{
			atkData = &data->atk;
		}
	}

	if (p_atkText)
	{
		FString ss = FString("");
		if (atkData)
		{
			ss = FString::Printf(TEXT("p_atk:%d"), atkData->p_atk);
		}
		p_atkText->SetText(FText::FromString(ss));
	}
	if (m_atkText)
	{
		FString ss = FString("");
		if (atkData)
		{
			ss = FString::Printf(TEXT("m_atk:%d"), atkData->m_atk);
		}
		m_atkText->SetText(FText::FromString(ss));
	}
	if (p_defendText)
	{
		FString ss = FString("");
		if (atkData)
		{
			ss = FString::Printf(TEXT("p_defend:%d"), atkData->p_defend);
		}
		p_defendText->SetText(FText::FromString(ss));
	}
	if (m_defendText)
	{
		FString ss = FString("");
		if (atkData)
		{
			ss = FString::Printf(TEXT("m_defend:%d"), atkData->m_defend);
		}
		m_defendText->SetText(FText::FromString(ss));
	}
	if (dodgeText)
	{
		FString ss = FString("");
		if (atkData)
		{
			ss = FString::Printf(TEXT("dodge:%d"), atkData->dodge);
		}
		dodgeText->SetText(FText::FromString(ss));
	}
	if (critText)
	{
		FString ss = FString("");
		if (atkData)
		{
			ss = FString::Printf(TEXT("crit:%d"), atkData->crit);
		}
		critText->SetText(FText::FromString(ss));
	}
}
