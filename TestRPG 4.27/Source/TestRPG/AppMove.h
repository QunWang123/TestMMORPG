#ifndef __APPMOVE_H
#define __APPMOVE_H

#include "IContainer.h"

namespace app
{
	class AppMove : public IContainer
	{
	public:
		AppMove();
		virtual ~AppMove();
		virtual void  onInit();
		virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd);

		static void send_Move(float _face, float _speed, FVector _pos, FVector _targetpos);
	};

	extern AppMove* __AppMove;
}


#endif