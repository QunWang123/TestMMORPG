// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "UI_Chat.generated.h"

class UTextBlock;
class UEditableText;
class UButton;
class UScrollBox;
/**
 * 
 */
UCLASS()
class TESTRPG_API UUI_Chat : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
		UTextBlock* DialogueReviewBlock;

	UPROPERTY(meta = (BindWidget))
		UScrollBox* DialogueReviewScroll;

	UPROPERTY(meta = (BindWidget))
		UEditableText* DialogueInputBlock;

	UPROPERTY(meta = (BindWidget))
		UButton* SendButton;

	// dialogreviewboxµÄ»º´æ
	FString StringCash;
	float time;

public:

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "UIChat")
		bool addText(FString InText);

	UFUNCTION()
	void sendMessage();

	UFUNCTION()
	void sendMessageCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};
