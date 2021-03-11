#include "../main/User.h"
#include "../contents/EventManager.h"
#include "PktEventDecreaseHandler.h"
#include "Protocol.h"


void PktEventDecreaseHandler::OnHandler( Packet& packet, User* user )
{
	if ( !user )
		return;

	LogicThread* logicThread = user->GetLogicThread();
	if ( !logicThread )
		logicThread = (LogicThread*)( ThreadManager::GetSingleton()->GetIdleThread( "LogicThread" ) );

	int key = 0;
	int value = 0;
	packet >> key >> value;
	logicThread->RunTask
	(
		[ key, value, user ] ()
		{
			EventManager::GetSingleton()->SetEventMapCount( key, value );

			Packet sendPacket( Protocol::EventDecreaseResult );
			sendPacket << (std::string)( "PktEventDecreaseHandler result" );
			user->SendPacket( sendPacket );
		}
	);

	
	
}
