// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintFunction.generated.h"

class AMyActor;

#pragma pack(push, packing)
#pragma pack(1)
USTRUCT(BlueprintType)
struct FPlayerBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlueprintV")
		int32 memid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlueprintV")
		int32 socketfd;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlueprintV")
		int32 state;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlueprintV")
		int32 curhp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlueprintV")
		int32 maxhp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlueprintV")
		float speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlueprintV")
		FVector pos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlueprintV")
		FRotator rot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlueprintV")
		FString nick;
};
#pragma pack(pop, packing)
/**
 * 
 */
UCLASS()
class TESTRPG_API UBlueprintFunction : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static int32 AppUpdate();

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void DisConnect();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static bool isSecurity();


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static bool isLogin();

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void InitHeartTime();

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static int32 GetDelayTime();							// 算ping

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static int32 read_int8();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static int32 read_int16();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static int32 read_int32();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static float read_float();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static bool read_bool();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static FVector read_FVector();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static FRotator read_FRotator();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static FString read_FString();			// 隐式解析

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static FString read_FString_len(int32 len);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static FPlayerBase read_FPlayerBase();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static FROLE_PROP read_FROLE_PROP();

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_begin(int32 cmd);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_end();



	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_int8(int32 value);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_int16(int32 value);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_int32(int32 value);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_float(float value);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_bool(bool value);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_FVector(FVector value);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_FRotator(FRotator value);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_FString(FString value);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_FString_len(FString value, int32 len);


	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_Login(FString name, int32 namelen, FString password, int32 passwordlen);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_Register(FString name, int32 namelen, FString password, int32 passwordlen);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_LoginGame(FString name, int32 namelen);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static FUSER_MEMBER_ROLE getRoleDataBase(int32 index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static FUSER_ROLE_BASE myUserData();

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_CreateRole(uint8 injob, uint8 insex, FString innick);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_DeleteRole(uint8 roleindex);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_StartGame(uint8 roleindex);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_EntryWorld();

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_GetOtherRoleData(int32 userindex);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_Move(float face, float speed, FVector pos, FVector targetpos);
	// send chat
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_ChatText(FString text);

	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static int32 getSkillScript_Distance(int32 id, int32 level);

	// 查找玩家显示对象
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static ACharacterBase* findUserView(int32 userindex);
	// 发送复活
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_Reborn(int32 kind);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static FVector GetMapLeftPos();

	// 发送获取掉落
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_GetDrop(int32 dropindex, int32 dropid);
	// 获取职业
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlueprintCF")
		static FString GetJob(int32 JonIndex);

	// 背包
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_GetBag();
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_GetCombat(int32 userindex);
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_CombaineBag();

	// 切换地图、线路
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_ChangeMap(int32 mapid);
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static void send_ChangeLine(uint8 line);
	// 获取地图名称
	UFUNCTION(BlueprintCallable, Category = "BlueprintCF")
		static FString GetMapName(int32 mapid);

	//地图编辑器部分
	//*******************************************************
	//设置最大行列
	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		static void editor_SetMaxRowCol(FVector left, FVector right);
	//坐标转换
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
		static FVector editor_UpdateNewPosition(FVector pos);
	//坐标转为网格
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
		static FGRID_BASE editor_PosToGrid(FVector pos);
	//网格转为坐标
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
		static FVector editor_GridToPos(FGRID_BASE grid);

	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		static void editor_SetCurState(int32 state);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
		static int32 editor_getCurState();

	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		static void editor_InsertActor(AMyActor* a);
	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		static void editor_DeleteActor(int row, int col);
	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		static void editor_DeleteAllSpecID(int id);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlowerEngine")
		static int32 editor_IsExistActor(int row, int col);

	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		static void editor_DestoryAll();


	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		static void editor_SaveName(FString filename);
	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		static void editor_OpenName(FString filename);
	UFUNCTION(BlueprintCallable, Category = "FlowerEngine")
		static TArray<FString> editor_GetMapData();
};
