// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuleBullet.generated.h"

UENUM(BlueprintType)
enum class EBulletType : uint8
{
	BULLET_NONE,			// 不产生任何效果

	BULLET_DIRECT_LINE,		// 无障碍直线攻击
	BULLET_LINE,			// 非跟踪类型，类似手枪攻击
	BULLET_TRACK_LINE,		// 跟踪类型
	BULLET_TRACK_LINE_SP,	// 跟踪类型
	BULLET_RANGE,			// 范围伤害, 类似地雷
	BULLET_RANGE_LINE,		// 范围伤害，类似手雷
	BULLET_CHAIN,			// 链条类型，持续伤害类型
};


UCLASS()
class TESTRPG_API ARuleBullet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARuleBullet();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = "true"))
		class USphereComponent* BoxDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = "true"))
		class USceneComponent* RootBullet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = "true"))
		class UProjectileMovementComponent* ProjectileMovement;	

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
		TEnumAsByte<EBulletType> BulletType;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
		class UParticleSystem* DamageParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
		class UParticleSystem* OpenFireParticle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// UFUNCTION()
	//	 void RangeExplosion(ARuleofTheCharacter* HitedCharacter, const FVector& HitLocation);

	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
