// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

APlayerCharacterBase::APlayerCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// FallingAttackAnimMontage->OnMontageEnded.AddDynamic(this, &APlayerCharacterBase::initEventing);
	
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	CharacterType = ECharacterType::PLAYER_NONE;
}

void APlayerCharacterBase::EndSkillMontagePlay(UAnimMontage* InMontage, bool bInterrupted)
{
	USkeletalMeshComponent* myMesh = GetMesh();
	UAnimInstance* AnimInstance = (myMesh) ? myMesh->GetAnimInstance() : nullptr;
	if (AnimInstance && InMontage)
	{
		// 走到这里说明对应InMontage的OnMontageEnded不再需要了，需要删除。
		// （不确定删除到底需不需要，假设只有技能需要播放蒙太奇的话，那不删应该也行）
		FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(InMontage);
		if (MontageInstance)
		{
			MontageInstance->OnMontageEnded.Unbind();
		}
	}
	// 动画播完了就走这里，自定义的初始任务事件，事件结束时就走这里
	initEventing(IsLocallyControlled());
}

float APlayerCharacterBase::PlayerPlaySkillAnimMontage(UAnimMontage* AnimMontage, float InPlayRate /*= 1.f*/, FName StartSectionName /*= NAME_None*/)
{
	USkeletalMeshComponent* myMesh = GetMesh();
	UAnimInstance* AnimInstance = (myMesh) ? myMesh->GetAnimInstance() : nullptr;
	if (AnimMontage && AnimInstance)
	{
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

		// 绑定完成事件动画播完就会走EndSkillMontagePlay事件
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &APlayerCharacterBase::EndSkillMontagePlay);
		AnimInstance->Montage_SetEndDelegate(EndDelegate);

		if (Duration > 0.f)
		{
			// Start at a given Section.
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}

			return Duration;
		}
	}
	return 0.f;
}
