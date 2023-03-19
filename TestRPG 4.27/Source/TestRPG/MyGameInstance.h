// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UserData.h"
#include "RobotData.h"
#include "MyGameInstance.generated.h"


/**
 * 
 */
UCLASS()
class TESTRPG_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MyEngine")
		int AppInitGameInstance();

	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		void onCommand(int cmd);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		void onConnect(int errcode);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		void onDisConnect(int errcode);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		void onSecureConnect(int errcode);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		void onExceptConnect(int errcode);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		bool onLoginCommand(int cmd, int childcmd);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		bool onSelectRoleCommand(int cmd, int childcmd, uint8 roleindex);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		bool onEntryWorldCommand(int cmd, int childcmd, int value);

	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		bool onOtherRoleDataCommand(int childcmd, FOTHER_ROLE_BASE data);
	// ��������ƶ�
	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		bool onMoveCommand(int userindex, int face, int speed, FVector targetpos);

	//�յ�������ϸ��Ϣ�¼�
	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		bool onRobotDataCommand(int32 childcmd, FROBOT_BASE data);

	// �յ�������Ϣ
	UFUNCTION(BlueprintImplementableEvent, Category = "FlowerEngine")
		bool onDropCommand(int32 childcmd, FDROP_BASE drop);

	// ����
	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		bool onBagCommand(int cmd, int32 userindex, uint8 bagpos, uint8 combatpos, uint8 kind);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "MyEngine")
		bool onChangeMapCommand(int32 cmd, int32 childcmd, int32 mapid);

	int32 GetTimeSeconds();

	virtual void Shutdown();

	// ��ͼ�༭������
	UFUNCTION(BlueprintImplementableEvent, Category = "MyEngine")
		void editor_LoadDataOver();
	
	// ���õ�������ʱ��
	// float time;
};

extern UMyGameInstance* __AppGameInstance;