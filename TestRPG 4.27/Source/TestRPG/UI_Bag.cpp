// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_Bag.h"
#include "UI_BagGrid.h"
#include "UI_BagGridSlot.h"
#include "UserData.h"
#include "BlueprintFunction.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI_CombatGrid.h"

void UUI_Bag::NativeConstruct()
{
	Super::NativeConstruct();
	updateEconomyInfo();
	if (ComBatGrid)
	{
		ComBatGrid->SetSlotsPos();
	}
}

void UUI_Bag::updateAllBagInfo()
{
	if (BagGrid)
	{
		// 这个还能根据背包大小新开空间
		BagGrid->LayoutInventorySlot();
	}
}

void UUI_Bag::updateAllCombatInfo()
{
	if (ComBatGrid)
	{
		ComBatGrid->updateAllCombatInfo(__myUserData.userindex);
		ComBatGrid->updateCombatAtk(__myUserData.userindex);
	}
}

void UUI_Bag::updateEconomyInfo()
{
	if (GoldsText)
	{
		FString ss = FString::Printf(TEXT("Golds:%d"), __myUserData.base.econ.gold);
		GoldsText->SetText(FText::FromString(ss));
	}
	if (DiamondsText)
	{
		FString ss = FString::Printf(TEXT("Diamonds:%d"), __myUserData.base.econ.diamonds);
		DiamondsText->SetText(FText::FromString(ss));
	}
}

void UUI_Bag::updateBagSpecifyGrid(uint8 index)
{
	if (BagGrid)
	{
		BagGrid->updateSpecifyGrid(index);
	}
}

void UUI_Bag::updateBagSpecifyGridNum(uint8 index)
{
	if (BagGrid)
	{
		BagGrid->updateSpecifyGridNum(index);
	}
}

void UUI_Bag::updateCombatSpecifyGrid(uint8 index)
{
	if (ComBatGrid)
	{
		if (index < 0 || index >= __myUserData.stand.combat.equips.Num()) return;
		ComBatGrid->updateCombatSpecifyGrid(index, __myUserData.stand.combat.equips[index]);
	}
}

void UUI_Bag::updateCombatAtk()
{
	if (ComBatGrid)
	{
		ComBatGrid->updateCombatAtk(__myUserData.userindex);
	}
}

void UUI_Bag::updateCombatHeadInfo()
{
	if (ComBatGrid)
	{
		ComBatGrid->updateHeadInfo(__myUserData.userindex);
	}
}

