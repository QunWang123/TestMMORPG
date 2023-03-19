// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlueprintFunction.h"
#include "UserData.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// AddMovementInput();
// 	if (IsLocallyControlled())
// 	{
// 		Speed = GetVelocity().Size();
// 	}
}
// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


bool ACharacterBase::isMove(bool ismy)
{
	if (!UBlueprintFunction::isLogin()) return false;

	if (ismy)
	{
		if (__myUserData.events.isEventing)
		{
			if (__myUserData.events.event_Type == (uint8)E_EVENT_TYPE::EVENT_SKILL)
			{
 				if (__myUserData.events.event_LockAtk.lock_SkillID < 1006) return true;
			}
			return false;
		}
		int32 value = __myUserData.tmp.buff_run.state & E_BUFF_NOMOVE;
		if (value > 0) return false;
		if (__myUserData.base.life.hp <= 0) return false;

		return true;
	}
	// 其他玩家
	if (this->userdata == NULL) return false;
	FOTHER_ROLE_BASE* d = this->userdata;
	if (d->events.isEventing)
	{
		if (d->events.event_Type == (uint8)E_EVENT_TYPE::EVENT_SKILL)
		{
			if (d->events.event_LockAtk.lock_SkillID < 1006) return true;
		}
		return false;
	}

	if (d->hp <= 0) return false;
	return true;
}

void ACharacterBase::updatePosition()
{
	if (userdata == NULL) return;
	this->userdata->pos = GetActorLocation();
}

void ACharacterBase::initEventing(bool ismy)
{
	if (ismy)
	{
		__myUserData.events.isEventing = false;
		__myUserData.events.event_Type = 0;
		__myUserData.events.event_LockAtk.reset();
	}
	else
	{
		if (userdata == NULL) return;
		userdata->events.isEventing = false;
		userdata->events.event_Type = 0;
		userdata->events.event_LockAtk.reset();
	}
}

//  设置显示，互相关联
void ACharacterBase::setView(int32 userindex)
{
	FOTHER_ROLE_BASE* d = onLineDatas.Find(userindex);
	if (d == nullptr) return;
	d->view = this;
	this->userdata = d;

}
// 返回攻击 事件数据
FLOCK_ATK_DATA ACharacterBase::getLockData()
{
	if (userdata == NULL)
	{
		FLOCK_ATK_DATA d;
		d.reset();
		return d;
	}
	return this->userdata->events.event_LockAtk;
}

int32 ACharacterBase::getEventCount()
{
	if (userdata == NULL) return -1;
	return this->userdata->events.event_List.Num();
}

