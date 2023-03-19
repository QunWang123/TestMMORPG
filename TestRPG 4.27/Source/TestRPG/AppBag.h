#ifndef __APPBAG_H
#define __APPBAG_H

#include "IContainer.h"

namespace app
{
	class AppBag : public IContainer
	{
	public:
		AppBag();
		virtual ~AppBag();
		virtual void  onInit();
		virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd);

		static void send_GetBag();
		static void send_GetCombat(int32 userindex);
		static void send_CombaineBag();
		static void send_EquipUp(int32 pos, int32 propid);
		static void send_EquipDown(int32 pos, int32 propid);
		static void send_ItemSell(int32 pos, int32 propid);
		static void send_Swap(int32 pos1, int32 propid1, int32 pos2, int32 propid2);
	};

	extern AppBag* __AppBag;
}


#endif

