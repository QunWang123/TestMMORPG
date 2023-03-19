#ifndef  ____APPDROP_H
#define  ____APPDROP_H

#include "IContainer.h"
namespace app
{
	class AppDrop : public IContainer
	{
	private:
		void OnGet(net::ITCPServer* ts, net::S_CLIENT_BASE* c);
	public:
		AppDrop();
		virtual ~AppDrop(); 
		virtual void  onUpdate();
		virtual bool  onServerCommand(net::ITCPServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);

	public:
		static void clearDrop(s32 mapid, s32 layer);
	};

	extern IContainer* __AppDrop;
}

#endif