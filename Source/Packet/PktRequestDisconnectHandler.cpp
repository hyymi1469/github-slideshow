#include "../main/User.h"
#include "../contents/EventManager.h"
#include "PktRequestDisconnectHandler.h"
#include "Protocol.h"


void PktRequestDisconnectHandler::OnHandler( Packet& packet, User* user )
{
	if ( !user )
		return;

	user->OnDestroy();
}
