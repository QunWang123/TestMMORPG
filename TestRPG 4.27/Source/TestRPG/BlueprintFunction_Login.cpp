#include "BlueprintFunction.h"
#include "AppLogin.h"
#include "AppSelectRole.h"
#include "AppEntryWorld.h"
#include "AppMove.h"

void UBlueprintFunction::send_Login(FString name, int32 namelen, FString password, int32 passwordlen)
{
	app::AppLogin::send_Login(name, namelen, password, passwordlen);
}

void UBlueprintFunction::send_Register(FString name, int32 namelen, FString password, int32 passwordlen)
{
	app::AppLogin::send_Register(name, namelen, password, passwordlen);
}

void UBlueprintFunction::send_LoginGame(FString name, int32 namelen)
{
	app::AppLogin::send_LoginGame(name, namelen);
}

FUSER_MEMBER_ROLE UBlueprintFunction::getRoleDataBase(int32 index)
{
	FUSER_MEMBER_ROLE role;
	FMemory::Memset(&role, 0, 22);
	if (index < 0 || index >= USER_MAX_ROLE) return role;
	return rolebase[index];
}

FUSER_ROLE_BASE UBlueprintFunction::myUserData()
{
	return __myUserData;
}

void UBlueprintFunction::send_CreateRole(uint8 injob, uint8 insex, FString innick)
{
	app::__AppSelectRole->send_CreateRole(injob, insex, innick);
}

void UBlueprintFunction::send_DeleteRole(uint8 roleindex)
{
	app::__AppSelectRole->send_DeleteRole(roleindex);
}

void UBlueprintFunction::send_StartGame(uint8 roleindex)
{
	app::__AppSelectRole->send_StartGame(roleindex);
}

void UBlueprintFunction::send_EntryWorld()
{
	app::__AppEntryWorld->send_EntryWorld();
}

void UBlueprintFunction::send_GetOtherRoleData(int32 userindex)
{
	app::__AppEntryWorld->send_GetOtherRoleData(userindex);
}

void UBlueprintFunction::send_Move(float face, float speed, FVector pos, FVector targetpos)
{
	app::__AppMove->send_Move(face, speed, pos, targetpos);
}
