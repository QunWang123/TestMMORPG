// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_Chat.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MyGameInstance.h"
#include "AppEntryWorld.h"
#include "Components/ScrollBox.h"
#include "BlueprintFunction.h"

void UUI_Chat::NativeConstruct()
{
	Super::NativeConstruct();
	// 绑定事件
	SendButton->OnReleased.AddDynamic(this, &UUI_Chat::sendMessage);
	DialogueInputBlock->OnTextCommitted.AddDynamic(this, &UUI_Chat::sendMessageCommitted);
	
	// 对话内容不可修改
	DialogueReviewBlock->bIsEnabled = 0;
	// 初始化时间
	if (__AppGameInstance)
	{
		time = __AppGameInstance->GetTimeSeconds();
	}
	// 初始化对话内容
	StringCash = "";
}

bool UUI_Chat::addText(FString InText)
{
	StringCash = StringCash + InText + "\r\n";

	if (!DialogueReviewBlock)
	{
		return false;
	}
	DialogueReviewBlock->SetText(FText::FromString(StringCash));
	DialogueReviewScroll->ScrollToEnd();
	return true;
}

void UUI_Chat::sendMessage()
{
	if (DialogueInputBlock)
	{
		FString sendText = DialogueInputBlock->GetText().ToString();
		if (sendText != "")
		{
			if (__AppGameInstance)
			{
				// 两次间隔时间大于1.5 秒
				if (__AppGameInstance->GetTimeSeconds() - time > 1500)
				{
					time = __AppGameInstance->GetTimeSeconds();
					UBlueprintFunction::send_ChatText(sendText);
					DialogueInputBlock->SetText(FText::FromString(""));
				}
			}
		}
	}
}

void UUI_Chat::sendMessageCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		sendMessage();
	}
}

