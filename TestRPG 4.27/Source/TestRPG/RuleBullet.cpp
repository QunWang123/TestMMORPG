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


// ������Ҿ�����ôд�����Լ��޹ص�Bullet���ɷ�������֪ͨ��
// ���Լ��йصòŻ������
void ARuleBullet::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// �ðɣ�����ú�����Լ��ҹ�����
	// �������__MyCharacter�ҹ����У��������Ҹ�
	if (__MyCharacter == Cast<AMyCharacterBase>(OtherActor))
	{
		if (__MyCharacter != Cast<AMyCharacterBase>(GetInstigator()))
		{
			if (ACharacterBase* DamageCauser = Cast<ACharacterBase>(GetInstigator()))
			{
				if (DamageCauser->CharacterType >= ECharacterType::PLAYER_ENEMY)
				{
					// ���ַ������ǵжԻ���monster
					// ���������ж��Ǽ�������ͣ
					ProjectileMovement->StopMovementImmediately();
					// ���ɼ�����Ч
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DamageParticle, __MyCharacter->GetActorLocation());
					// �����˺������ͷ�����
				}
			}
		}
	}
	else
	{
		// �Լ������ܻ���,���Ƿ�����
		if (__MyCharacter == Cast<AMyCharacterBase>(GetInstigator()))
		{
			// ��ȡ�ܻ���
			if (ACharacterBase* HittedCharacter = Cast<ACharacterBase>(OtherActor))
			{
				if (HittedCharacter->CharacterType >= ECharacterType::PLAYER_ENEMY)
				{
					// ���ַ������ǵжԻ���monster
					// ���������ж��Ǽ�������ͣ
					ProjectileMovement->StopMovementImmediately();
					// ���ɼ�����Ч
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DamageParticle, HittedCharacter->GetActorLocation());
					// �����˺������ͷ�����
				}
			}
		}
	}
}
