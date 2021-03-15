#include "../main/User.h"
#include "../contents/EventManager.h"
#include "PktEventIncreaseHandler.h"
#include "Protocol.h"


void PktEventIncreaseHandler::OnHandler( Packet& packet, User* user )
{
	if ( !user )
		return;

	LogicThread* logicThread = user->GetLogicThread();
	if ( !logicThread )
		logicThread = (LogicThread*)( ThreadManager::GetSingleton()->GetIdleThread( "LogicThread" ) );

	int recvTest1 = 0;
	int recvTest2 = 0;
	packet >> recvTest1 >> recvTest2;
	
	// mustex 락 테스트
	/*
	int size = 0;
	{
		std::unique_lock< std::mutex > lock( m_mutex );

		size = EventManager::GetSingleton()->GetTestMapSize() + 1;
		EventManager::GetSingleton()->EmplaceTestMap( size, size );
		if ( size >= 10000 )
			printf( "Complete Time!! : %f\nserverFinish!\n", EventManager::GetSingleton()->GetTestTime() );
	}

	Packet sendPacket( Protocol::EventIncreaseResult );
	sendPacket << (std::string)( "PktEventIncreaseHandler test result" ) << size;
	user->SendPacket( sendPacket );
	*/
	
	
	// lock-free 테스트
	logicThread->RunTask
	(
		[ user ] ()
		{
			int size = EventManager::GetSingleton()->GetTestMapSize() + 1;
			EventManager::GetSingleton()->EmplaceTestMap( size, size );
			if ( size >= 10000 )
				printf( "Complete Time!! : %f\nserverFinish!\n", EventManager::GetSingleton()->GetTestTime() );

			for ( auto iter : EventManager::GetSingleton()->GetTestMap() )
			{
			}

			Packet sendPacket( Protocol::EventIncreaseResult );
			sendPacket << (std::string)( "PktEventIncreaseHandler test result" ) << size;
			user->SendPacket( sendPacket );

		}
	);
}
