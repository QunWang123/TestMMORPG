#include "MyGameUtils.h"
#include "Engine/World.h"

ARuleBullet* MyGameUtils::SpawnBullet(UWorld* InWorld, APawn* NewPawn, UClass* InClass, const FVector& Loc, const FRotator& Rot)
{
	if (InWorld && InClass && NewPawn)
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Instigator = NewPawn;

		if (ARuleBullet* Bullet = InWorld->SpawnActor<ARuleBullet>(InClass, Loc, Rot, ActorSpawnParameters))
		{
			return Bullet;
		}
	}
	return nullptr;
}

