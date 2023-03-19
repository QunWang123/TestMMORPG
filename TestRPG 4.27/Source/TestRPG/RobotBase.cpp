// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotBase.h"

// Sets default values
ARobotBase::ARobotBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	rindex = -1;
}

// Called when the game starts or when spawned
void ARobotBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARobotBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARobotBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ARobotBase::initEventing(int32 index)
{
	FROBOT* data = robot_Find(index);
	if (data == NULL) return;

	data->events.isEventing = false;
	data->events.event_Type = 0;
	data->events.event_LockAtk.reset();
}

void ARobotBase::setView(int32 robotindex)
{
	FROBOT* data = robot_Find(robotindex);
	if (data == NULL) return;
	data->view = this;
	rindex = data->data.robotindex;
	robotView_Add(robotindex, this);
}

void ARobotBase::destroyRobot()
{
	robotView_Remove(rindex);
	robot_Remove(rindex);
	this->Destroy();
}

FLOCK_ATK_DATA ARobotBase::getLockData()
{
	FROBOT* data = robot_Find(rindex);
	check(data != NULL);
	
	return data->events.event_LockAtk;
}

