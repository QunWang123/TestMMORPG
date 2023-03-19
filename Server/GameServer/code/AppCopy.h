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
	//�����������ݽṹ
	struct S_COPY_ROOM
	{
		s32   index;//��������
		u8    state;   //��ǰ״̬
		u32   mapid;   //��ͼID
		u8    maptype; //��ͼ����
		u8    curnum;  //��ǰ����
		u8    maxnum;  //�������
		s32   limitime;//����ʱ��
		//s32   endtime; //����ʱ��
		s32   teamindex;//�����±��
		s32   monstercount;//��������
		s32   userindex;//���˸����µ� �������
		u64   memid;//���˸�����¼��ҵ��˺�ID
		s64   temp_time;//���ڼ�¼ʱ��
		s64   temp_teamCreatetime;//���鴴��ʱ��


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
		HashArray<S_COPY_ROOM>* __Rooms;//��������
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