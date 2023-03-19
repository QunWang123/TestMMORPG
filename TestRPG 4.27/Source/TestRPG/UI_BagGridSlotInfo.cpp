// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_BagGridSlotInfo.h"
#include "UserData.h"
#include "Components/TextBlock.h"

void UUI_BagGridSlotInfo::InitInfo(const FROLE_PROP& InData)
{
	FString ss = "id: " + FString::FromInt(InData.base.id) + "\r\n";
	ss = ss + "count: " + FString::FromInt(InData.base.count) + "\r\n";
	if (InData.equip.p_atk > 0)
	{
		ss = ss + "p_atk+" + FString::FromInt(InData.equip.p_atk) + "\r\n";
	}
	if (InData.equip.m_atk > 0)
	{
		ss = ss + "m_atk+" + FString::FromInt(InData.equip.m_atk) + "\r\n";
	}
	if (InData.equip.p_defend > 0)
	{
		ss = ss + "p_defend+" + FString::FromInt(InData.equip.p_defend) + "\r\n";
	}
	if (InData.equip.m_defend > 0)
	{
		ss = ss + "m_defend+" + FString::FromInt(InData.equip.m_defend) + "\r\n";
	}
	if (InData.equip.crit > 0)
	{
		ss = ss + "crit+" + FString::FromInt(InData.equip.crit) + "\r\n";
	}
	if (InData.equip.dodge > 0)
	{
		ss = ss + "dodge+" + FString::FromInt(InData.equip.dodge) + "\r\n";
	}
	if (InData.equip.hp > 0)
	{
		ss = ss + "hp+" + FString::FromInt(InData.equip.hp) + "\r\n";
	}
	if (InData.equip.mp > 0)
	{
		ss = ss + "mp+" + FString::FromInt(InData.equip.mp) + "\r\n";
	}
	ss = ss + "owner: " + InData.equip.nick + "\r\n";
	ss = ss + "money: " + FString::FromInt(InData.base.money) + "\r\n";
	SlotItemID->SetText(FText::FromString(ss));
}
