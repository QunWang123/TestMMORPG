// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UserData.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	PLAYER_NONE,			// ���������Ĭ��״̬
	PLAYER_TEAM,			// ��Ҷ���
	PLAYER_ENEMY,			// ��ҵ���
	MONSTER_SIMPLE,
	MONSTER_BOSS,
};

UCLASS()
class TESTRPG_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditDefaultsOnly, Category = "CharacterType")
		TEnumAsByte<ECharacterType> CharacterType;

	

public:

	//�������
	FOTHER_ROLE_BASE*  userdata;

	
// 	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Speed")
// 		float Speed;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//�ж��ǲ��ǿ����ƶ�
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CharacterBase")
		bool isMove(bool ismy);
	UFUNCTION(BlueprintCallable, Category = "CharacterBase")
		void updatePosition();

	//1 ʹ�ü���
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onSkillCommand_Play();
	//2 �յ�buff
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onBuffCommand_Play(int32 id, int32 runningtime);
	//3 ʹ�ü��� �յ��˺�ֵ
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "CharacterBase")
		bool onSkillCommand_HP(int32 curhp, int32 atk);
	//4 �յ��������
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onCommand_Dead();
	//����
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onRebornCommand_Play(int32 err, FVector pos);

	//����
	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterBase")
		bool onCommand_Level(int32 level);

	//��ʼ�� �¼�
	UFUNCTION(BlueprintCallable, Category = "CharacterBase")
		void initEventing(bool ismy);

	//�ຯ�� �̳�����Ե��� ��ͼ����
	UFUNCTION(BlueprintCallable, Category = "CharacterBase")
		void setView(int32 userindex);

 	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
 		FLOCK_ATK_DATA getLockData();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CharacterBase")
		int32 getEventCount();
};
