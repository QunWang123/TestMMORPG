// Fill out your copyright notice in the Description page of Project Settings.


#include "OtherPlayerCharacterBase.h"

void AOtherPlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	CharacterType = ECharacterType::PLAYER_ENEMY;
}
