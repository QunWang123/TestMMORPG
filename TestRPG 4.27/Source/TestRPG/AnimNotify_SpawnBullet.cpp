// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SpawnBullet.h"
#include "Components/SkeletalMeshComponent.h"
#include "RuleBullet.h"
#include "MyGameUtils.h"
#include "MyCharacterBase.h"

UAnimNotify_SpawnBullet::UAnimNotify_SpawnBullet()
	:Super()
{
#if WITH_EDITORONLY_DATA
	InSocketName = TEXT("FirePoint");
	NotifyColor = FColor(196, 142, 255, 255);
#endif // WITH_EDITORONLY_DATA
}

FString UAnimNotify_SpawnBullet::GetNotifyName_Implementation() const
{
	if (BulletClass)
	{
		return BulletClass->GetName();
	}
	else
	{
		return Super::GetNotifyName_Implementation();
	}
}

void UAnimNotify_SpawnBullet::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
// #if WITH_EDITOR
// 	FTransform CharacterTransform = MeshComp->GetSocketTransform(InSocketName);
// #else
// 	FTransform CharacterTransform = Character->GetTransform();
// 	// 	if (ARuleofTheCharacter* Character = Cast<ARuleofTheCharacter>(MeshComp->GetOuter()))
// #endif

	// Cast<AMyCharacterBase>(GetOuter());
	// Cast<OTHER>(GetOuter());

	if (AActor* Character = Cast<AActor>(MeshComp->GetOuter()))
	{
		if (ARuleBullet* InBullet = MyGameUtils::SpawnBullet(Character->GetWorld(), Cast<APawn>(Character), BulletClass, Character->GetActorLocation(), Character->GetActorRotation()))
		{
			// InBullet->SubmissionSkillRequestType = ESubmissionSkillRequestType::MANUAL;

			// InitSkill其实就是把构造里面的东西考出来
			// InBullet->InitSkill();
			// InBullet->SubmissionSkillRequest();
			// InBullet->SetSubmissionDataType();
		}
	}
}