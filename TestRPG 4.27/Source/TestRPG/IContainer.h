#ifndef  __ICONTAINER_H
#define  __ICONTAINER_H

#include "TCPClient.h"

class IContainer
{
public:
	IContainer() {}
	virtual ~IContainer() {}
	virtual void  onInit() {}
	virtual void  onUpdate() {}
	virtual bool  onClientCommand(net::TCPClient* tc, const uint16 cmd) { return false; }
};


#endif // ! __ICONTAINER_H