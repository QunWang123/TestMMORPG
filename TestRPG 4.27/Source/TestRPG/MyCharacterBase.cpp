// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "WorldTools.h"
#include "AppManager.h"
#include "MyGameInstance.h"
#include "WorldScript.h"
#include "AppSkill.h"
#include "TimerManager.h"
#include "BlueprintFunction.h"
#include "UI_Bag.h"
#include "AppBag.h"

AMyCharacterBase* __MyCharacter = nullptr;

AMyCharacterBase::AMyCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	EMySpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("MySpringArm"));
	EMySpringArmComponent->SetupAttachment(RootComponent);
	EMySpringArmComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	EMySpringArmComponent->TargetArmLength = 400.0f;
	EMySpringArmComponent->CameraRotationLagSpeed = 15.0f;
	EMySpringArmComponent->CameraLagSpeed = 3.0f;

	EMyCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("MyCamera"));
	EMyCameraComponent->SetupAttachment(EMySpringArmComponent);

	EMyRadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("MyRadialForce"));
	EMyRadialForceComponent->SetupAttachment(RootComponent);
	EMyRadialForceComponent->SetAutoActivate(false);

	// JumpMaxCount = 2;
	LocalSkillIndex = -1;
	RotSpeed = 45.0f;
}

void AMyCharacterBase::BeginPlay()
{
	Super::BeginPlay();

}

void AMyCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!Velocity.IsZero())
	{
		FRotator Rotaion(0.0f, GetControlRotation().Yaw, 0.0f);
		GetCharacterMovement()->AddInputVector(UKismetMathLibrary::GetForwardVector(Rotaion) * Velocity.X);
		GetCharacterMovement()->AddInputVector(UKismetMathLibrary::GetRightVector(Rotaion) * Velocity.Y);
		
// 		AddMovementInput(UKismetMathLibrary::GetForwardVector(Rotaion), Velocity.X);
// 		AddMovementInput(UKismetMathLibrary::GetRightVector(Rotaion), Velocity.Y);
	}
}

void AMyCharacterBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacterBase::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &AMyCharacterBase::StopJumping);
	InputComponent->BindAction("SimpleAttack", IE_Pressed, this, &AMyCharacterBase::SimpleAttack);
	InputComponent->BindAction("SpecialAttack", IE_Pressed, this, &AMyCharacterBase::SpecialAttack);

	InputComponent->BindAxis("MoveForward", this, &AMyCharacterBase::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMyCharacterBase::MoveRight);
	InputComponent->BindAxis("Turn", this, &AMyCharacterBase::Turn);
	InputComponent->BindAxis("LookUp", this, &AMyCharacterBase::LookUp);
	InputComponent->BindAxis("Near", this, &AMyCharacterBase::SetSpringArmLength);
}

void AMyCharacterBase::MoveForward(float Val)
{
	if (isMove(true))
	{
		Velocity.X = FMath::Clamp(Val, -1.0f, 1.0f) * 10.0f;
	}
	else
	{
		Velocity.X = 0.f;
	}
}

void AMyCharacterBase::MoveRight(float Val)
{
	if (isMove(true))
	{
		Velocity.Y = FMath::Clamp(Val, -1.0f, 1.0f) * 10.0f;
	}
	else
	{
		Velocity.Y = 0.f;
	}
}

void AMyCharacterBase::Turn(float Val)
{
	AddControllerYawInput(Val * RotSpeed * UGameplayStatics::GetWorldDeltaSeconds(GetWorld()));
}

void AMyCharacterBase::LookUp(float Val)
{
	AddControllerPitchInput(-Val * RotSpeed * UGameplayStatics::GetWorldDeltaSeconds(GetWorld()));
}

void AMyCharacterBase::SetSpringArmLength(float Val)
{
	EMySpringArmComponent->TargetArmLength = UKismetMathLibrary::Lerp(
		EMySpringArmComponent->TargetArmLength,
		FMath::Clamp(EMySpringArmComponent->TargetArmLength - Val * 50.0f, 200.0f, 600.0f), 1.0f);
}
// #pragma optimize( "", off )
void AMyCharacterBase::Attack(TArray<UAnimMontage*> &InputAnimMontages, EAttackType InAttackType)
{
	if (InputAnimMontages.Num() == 0)
	{
		return;
	}

	if (InAttackType == CurrentAttackType)
	{
		if (IsAttacking)
		{
			SaveAttackNum = true;
		}
		else
		{
			IsAttacking = true;
			if (GetCharacterMovement()->IsFalling())
			{
				if (InAttackType == EAttackType::SIMPLEATTACK)
				{
					// PlayAnimMontage(FallingAttackAnimMontage);
					LocalSkillIndex = 2;
					// 调用函数
					useSkill_BP();
				}
				else
				{
					LocalSkillIndex = 5;
					useSkill_BP();
				}
				AttackNum = 0;
			}
			else
			{
				// PlayAnimMontage(InputAnimMontages[AttackNum]);
				if (InAttackType == EAttackType::SIMPLEATTACK)
				{
					LocalSkillIndex = AttackNum;
				}
				else
				{
					LocalSkillIndex = AttackNum + 3;
				}
				AttackNum += 1;
				AttackNum %= InputAnimMontages.Num();
				useSkill_BP();
			}
		}
	}
}

void AMyCharacterBase::SimpleAttack()
{
	if (CurrentAttackType != EAttackType::SPECIALATTACK)			// 人物处于NONE或者SIMPLEATTACK
	{
		CurrentAttackType = EAttackType::SIMPLEATTACK;
		Attack(MovingAttackAnimMontages, EAttackType::SIMPLEATTACK);
	}
}

void AMyCharacterBase::SpecialAttack()
{
	if (CurrentAttackType != EAttackType::SIMPLEATTACK)
	{
		CurrentAttackType = EAttackType::SPECIALATTACK;
		Attack(MovingSpeicalAnimMontages, EAttackType::SPECIALATTACK);
	}
}
// 保持连击，这里是由动画蓝图调用得
void AMyCharacterBase::SaveAttack()
{
	if (SaveAttackNum)
	{
		SaveAttackNum = false;
		IsAttacking = false;

		// Attack(MovingAttackAnimMontages, CurrentAttackType);

		if (CurrentAttackType == EAttackType::SIMPLEATTACK)
		{
			Attack(MovingAttackAnimMontages, EAttackType::SIMPLEATTACK);
		}
		else
		{
			Attack(MovingSpeicalAnimMontages, EAttackType::SPECIALATTACK);
		}
	}
}
// 连击失败，重算连击
void AMyCharacterBase::ResetAttack()
{
	IsAttacking = false;
	CurrentAttackType = EAttackType::NONE;
	SaveAttackNum = false;
	AttackNum = 0;
}

void AMyCharacterBase::ResetProjectileActivate()
{
	EMyRadialForceComponent->SetActive(false, false);
	GetWorld()->GetTimerManager().PauseTimer(TimerHandle);
}






void AMyCharacterBase::SetCharacter(AMyCharacterBase* player)
{
	__MyCharacter = player;
	setLockTarget(E_NODE_TYPE::N_FREE, -1, FVector::ZeroVector);
	__myUserData.tmp.reset();
	__myUserData.events.reset();
}


bool AMyCharacterBase::isMainRect(FVector tarpos)
{
	FVector v = GetActorLocation();
	FGRID_BASE grid;
	FGRID_BASE targrid;
	posToGridMax(&grid, &v, &__CurMapInfo.leftpos);
	posToGridMax(&targrid, &tarpos, &__CurMapInfo.leftpos);

	return isNineRect(&grid, &targrid);
}

void AMyCharacterBase::setLockTarget(E_NODE_TYPE lock_type, int32 lock_index, FVector targetpos)
{
	__myUserData.tmp.lock_Atk.lock_Type = (uint8)lock_type;
	__myUserData.tmp.lock_Atk.lock_Index = lock_index;
	__myUserData.tmp.lock_Atk.lock_Targetpos = targetpos;
}


int32 AMyCharacterBase::isUseSkill(int32 skillindex)
{
	// 验证是不是在处理其他事件

	if (__myUserData.base.life.hp < 0) return 5010;

	// 验证是不是受BUFF影响

	// 验证登录状态
	uint8 state = app::__TcpClient->getData()->state;
	if (state != func::C_LOGIN) return 5001;
	// 验证技能下标
	if (skillindex < 0 || skillindex >= MAX_SKILL_COUNT) return 5002;
	// 公共时间
	int32 ftime = __AppGameInstance->GetTimeSeconds() - __myUserData.tmp.skillPublicTime;
	if (ftime > 0 && ftime < 300) return 5003;
	// 验证冷却时间
	FROLE_STAND_SKILL_BASE* skill = &__myUserData.stand.myskill.skill[skillindex];
	script::SCRIPT_SKILL_BASE* skillscript = (skill == nullptr) ? nullptr : script::fingScript_Skill(skill->id, skill->level);
	if (skillscript == nullptr) return 5004;
	ftime = __AppGameInstance->GetTimeSeconds() - __myUserData.tmp.skillColdTime[skillindex];
	if (ftime < skillscript->coldtime) return 5005;
	// 验证职业
	if (__myUserData.rolebase.job != skillscript->needjob) return 5006;
	if (__myUserData.base.life.mp < skillscript->needmp) return 5012;
	// 验证攻击距离
	switch (__myUserData.tmp.lock_Atk.lock_Type)
	{
	case (uint8)E_NODE_TYPE::N_ROLE:
		{
		FOTHER_ROLE_BASE* data = onLineDatas.Find(__myUserData.tmp.lock_Atk.lock_Index);
		if (data != NULL)
		{
			if (data->hp <= 0) return 5013;
		}

		int32 distence = FVector::Dist(this->GetActorLocation(), __myUserData.tmp.lock_Atk.lock_Targetpos);
		if (distence > skillscript->distance - 30) return 5007;		// 距离不够
		if (__myUserData.tmp.lock_Atk.lock_Index == __myUserData.userindex) return 5008;
		FString ss = FString::Printf(TEXT("user skill distance ...%d-%d"), distence, skillscript->distance);
		if (GEngine) GEngine->AddOnScreenDebugMessage((uint64)-1, 55, FColor::Red, ss);
		}
		break;
	case (uint8)E_NODE_TYPE::N_MONSTER:
		break;
	case (uint8)E_NODE_TYPE::N_FREE:
		if (skillscript->islock != 0) return 5009;
		break;
	}

	return 0;
}

int32 AMyCharacterBase::sendSkill(int32 skillindex)
{
	__myUserData.tmp.skillColdTime[skillindex] = __AppGameInstance->GetTimeSeconds();
	__myUserData.tmp.skillPublicTime = __AppGameInstance->GetTimeSeconds();

	// 发送服务器
	app::AppSkill::send_Skill(skillindex,
		__myUserData.tmp.lock_Atk.lock_Type,
		__myUserData.tmp.lock_Atk.lock_Index,
		__myUserData.tmp.lock_Atk.lock_Targetpos);
	return 0;
}

// 判断能否跟随攻击，判断距离是不是足够
int32 AMyCharacterBase::isFollowAtk(int32 skillindex)
{
	// 验证技能下标
	if (skillindex < 0 || skillindex >= MAX_SKILL_COUNT) return 1;
	FROLE_STAND_SKILL_BASE* skill = &__myUserData.stand.myskill.skill[skillindex];
	script::SCRIPT_SKILL_BASE* skillscript = (skill == nullptr) ? nullptr : script::fingScript_Skill(skill->id, skill->level);
	if (skillscript == nullptr) return 2;

	if (__myUserData.tmp.lock_Atk.lock_Type != (uint8)E_NODE_TYPE::N_ROLE) return 3;

	auto view = UBlueprintFunction::findUserView(__myUserData.tmp.lock_Atk.lock_Index);
	if (view == NULL) return 4;

	int32 distance = FVector::Dist(this->GetActorLocation(), view->GetActorLocation());
	if (distance > skillscript->distance - 30) return 5;

	return 0;	
}
