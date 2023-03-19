// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_IconDragDrop.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Internationalization/Text.h"

void UUI_IconDragDrop::DrawICON(UTexture2D* InICON)
{
	ICON->SetBrushFromTexture(InICON);
}

void UUI_IconDragDrop::DrawItemID(FText inText)
{
	ItemID->SetText(inText);

}
