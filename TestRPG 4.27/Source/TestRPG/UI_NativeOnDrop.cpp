// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_NativeOnDrop.h"
#include "Blueprint/DragDropOperation.h"
#include "UI_BagGridSlot.h"

bool UUI_NativeOnDrop::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	bool bDrop = false;

	if (UDragDropOperation* StudingDragDropOperation = Cast<UDragDropOperation>(InOperation))
	{
		if (UUI_BagGridSlot* DraggedSlot = Cast<UUI_BagGridSlot>(InOperation->Payload))
		{
			DraggedSlot->ShowSlot();
			bDrop = true;
		}
	}
	return bDrop;
}