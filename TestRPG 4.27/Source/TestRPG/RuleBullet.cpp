// Fill out your copyright notice in the Description page of Project Settings.


#include "RuleBullet.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MyCharacterBase.h"
#include "PlayerCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "OtherPlayerCharacterBase.h"

// Sets default values
ARuleBullet::ARuleBullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxDamage = CreateDefaultSubobject<USphereComponent>(TEXT("BulletBoxDamage"));
	RootBullet = CreateDefaultSubobject<USceneComponent>(TEXT("BulletRootBullet"));
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("BulletProjectileMovement"));
	
	RootComponent = RootBullet;

	BoxDamage->AttachToComponent(RootBullet, FAttachmentTransformRules::KeepRelativeTransform);

	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->InitialSpeed = 1600.f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->UpdatedComponent = RootBullet;

	BulletType = EBulletType::BULLET_DIRECT_LINE;

	InitialLifeSpan = 4.0f;
}

// Called when the game starts or when spawned
void ARuleBullet::BeginPlay()
{
	Super::BeginPlay();
	BoxDamage->OnComponentBeginOverlap.AddDynamic(this, &ARuleBullet::BeginOverlap);
}

// Called every frame
void ARuleBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// 这个姑且就先这么写，与自己无关得Bullet就由服务器来通知吧
// 与自己有关得才会走这个
void ARuleBullet::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// 好吧，必须得和玩家自己挂钩才行
	// 即必须和__MyCharacter挂钩才行，否则不能乱搞
	if (__MyCharacter == Cast<AMyCharacterBase>(OtherActor))
	{
		if (__MyCharacter != Cast<AMyCharacterBase>(GetInstigator()))
		{
			if (ACharacterBase* DamageCauser = Cast<ACharacterBase>(GetInstigator()))
			{
				if (DamageCauser->CharacterType >= ECharacterType::PLAYER_ENEMY)
				{
					// 发现发出者是敌对或是monster
					// 根据设置判断是继续还是停
					ProjectileMovement->StopMovementImmediately();
					// 生成技能特效
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DamageParticle, __MyCharacter->GetActorLocation());
					// 计算伤害，发送服务器
				}
			}
		}
	}
	else
	{
		// 自己不是受击者,但是发出者
		if (__MyCharacter == Cast<AMyCharacterBase>(GetInstigator()))
		{
			// 获取受击者
			if (ACharacterBase* HittedCharacter = Cast<ACharacterBase>(OtherActor))
			{
				if (HittedCharacter->CharacterType >= ECharacterType::PLAYER_ENEMY)
				{
					// 发现发出者是敌对或是monster
					// 根据设置判断是继续还是停
					ProjectileMovement->StopMovementImmediately();
					// 生成技能特效
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DamageParticle, HittedCharacter->GetActorLocation());
					// 计算伤害，发送服务器
				}
			}
		}
	}
}
