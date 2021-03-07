#include "../main/User.h"
#include "../contents/EventManager.h"
#include "PktEventIncreaseHandler.h"
#include "Protocol.h"


void PktEventIncreaseHandler::OnHandler( Packet& packet, User* user )
{
	if ( !user )
		return;

	int key   = 0;
	int value = 0;
	packet >> key >> value;

	//printf( "key : %d, value : %d\n", key, value );

	EventManager::GetSingleton()->SetEventMapCount( key, value );

	Packet sendPacket( Protocol::EventIncreaseResult );
	sendPacket << (std::string)( "PktEventIncreaseHandler result" );
	user->SendPacket( sendPacket );
}