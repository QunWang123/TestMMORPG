#ifndef  ____APPCOPY_H
#define  ____APPCOPY_H

#include  "APPGlobal.h"
#include "WorldData.h"
#include "CommandData2.h"
namespace app
{
	enum E_COPY_ROOM_STATE
	{
		ECRS_FREE = 0x00,
		ECRS_GAMING = 0x01,
		ECRS_GAMECOMPLETED = 0x02,
		ECRS_GAMEFALIED = 0x03
	};
#pragma pack(push, packing)
#pragma pack(1)
	//副本房间数据结构
	struct S_COPY_ROOM
	{
		s32   index;//房间索引
		u8    state;   //当前状态
		u32   mapid;   //地图ID
		u8    maptype; //地图类型
		u8    curnum;  //当前人数
		u8    maxnum;  //最大人数
		s32   limitime;//限制时间
		//s32   endtime; //结束时间
		s32   teamindex;//队伍下标号
		s32   monstercount;//怪物数量
		s32   userindex;//单人副本下的 玩家索引
		u64   memid;//单人副本记录玩家的账号ID
		s64   temp_time;//用于记录时间
		s64   temp_teamCreatetime;//队伍创建时间


		inline void reset()
		{
			memset(this, 0, sizeof(S_COPY_ROOM));
			index = -1;
			teamindex = -1;
			userindex = -1;
		}
	};
#pragma pack(pop, packing)

	class  AppCopy :public IContainer
	{
	private:
		bool isUpdate;
		HashArray<S_COPY_ROOM>* __Rooms;//副本房间
	public:
		s32  roomCount;

		AppCopy();
		virtual ~AppCopy();
		virtual void  onInit();
		virtual void  onUpdate();
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  onClientCommand(net::ITCPClient* tc, const u16 cmd);
	public:
		void reset();
		S_COPY_ROOM* findEmpty();
		inline S_COPY_ROOM* findRoom(const s32 id)
		{
			if (id >= __Rooms->length || id < 0) return nullptr;
			return __Rooms->Value(id);
		}

		S_COPY_ROOM* findRoom(const s64 memid,s8& mateindex);
	
		static void startCopy(S_COPY_ROOM* room, S_USER_BASE* user, const u32 mapid);
		static void clearCopy(S_COPY_ROOM* room);
	};


	extern AppCopy*  __AppCopy;
	
}
#endif