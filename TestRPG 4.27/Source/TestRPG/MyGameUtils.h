#include "RuleBullet.h"
#ifndef __MYGAMEUTILS_H
#define __MYGAMEUTILS_H


namespace MyGameUtils
{
	ARuleBullet* SpawnBullet(UWorld* InWorld, APawn* NewPawn, UClass* InClass, const FVector& Loc, const FRotator& Rot);

}



#endif // __MYGAMEUTILS_H
