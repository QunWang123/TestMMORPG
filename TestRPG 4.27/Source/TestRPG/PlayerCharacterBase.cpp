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
		// �ߵ�����˵����ӦInMontage��OnMontageEnded������Ҫ�ˣ���Ҫɾ����
		// ����ȷ��ɾ�������費��Ҫ������ֻ�м�����Ҫ������̫��Ļ����ǲ�ɾӦ��Ҳ�У�
		FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(InMontage);
		if (MontageInstance)
		{
			MontageInstance->OnMontageEnded.Unbind();
		}
	}
	// ���������˾�������Զ���ĳ�ʼ�����¼����¼�����ʱ��������
	initEventing(IsLocallyControlled());
}

float APlayerCharacterBase::PlayerPlaySkillAnimMontage(UAnimMontage* AnimMontage, float InPlayRate /*= 1.f*/, FName StartSectionName /*= NAME_None*/)
{
	USkeletalMeshComponent* myMesh = GetMesh();
	UAnimInstance* AnimInstance = (myMesh) ? myMesh->GetAnimInstance() : nullptr;
	if (AnimMontage && AnimInstance)
	{
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

		// ������¼���������ͻ���EndSkillMontagePlay�¼�
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
