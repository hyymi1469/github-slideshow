#include <string>
#include "../main/User.h"
#include "../contents/EventManager.h"
#include "PktTestHandler.h"
#include "Protocol.h"


void PktTestHandler::OnHandler( Packet& packet, User* user )
{
	if ( !user )
		return;

	int aa = 0;
	std::string str = "";
	short bb = 0;
	Packet recvPacket = packet;

	recvPacket >> aa >> str >> bb;

	Packet sendPacket( Protocol::TestResult );
	sendPacket << (short)119 << (std::string)( "resultplet" ) << int( 999 );
	user->SendPacket( sendPacket );
}
