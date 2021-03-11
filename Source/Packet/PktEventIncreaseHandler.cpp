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

	int key = 0;
	int value = 0;
	packet >> key >> value;
	
	// mustex 락 테스트
	/*
	{
		std::unique_lock< std::mutex > lock( m_mutex );

		int size = EventManager::GetSingleton()->m_testEventMap.size() + 1;
			EventManager::GetSingleton()->m_testEventMap.emplace( size, size );

			printf( "curTestCount : %d / 10000\n", EventManager::GetSingleton()->m_testEventMap.size() );
			for ( auto iter : EventManager::GetSingleton()->m_testEventMap )
			{

			}

			if ( size >= 10000 )
			{
				printf( "Complete Time!! : %f\nserverFinish!\n", EventManager::GetSingleton()->GetTestTime() );
				User* testUser = nullptr;
				testUser->GetPort();
			}

			Packet sendPacket( Protocol::EventIncreaseResult );
			sendPacket << (std::string)( "PktEventIncreaseHandler result" );
			user->SendPacket( sendPacket );
	}
	*/
	
	
	// lock-free 테스트
	logicThread->RunTask
	(
		[ key, value, user ] ()
		{
			int size = EventManager::GetSingleton()->m_testEventMap.size() + 1;
			EventManager::GetSingleton()->m_testEventMap.emplace( size, size );

			printf( "curTestCount : %d / 10000\n", EventManager::GetSingleton()->m_testEventMap.size() );
			for ( auto iter : EventManager::GetSingleton()->m_testEventMap )
			{

			}

			if ( size >= 10000 )
			{
				printf( "Complete Time!! : %f\nserverFinish!\n", EventManager::GetSingleton()->GetTestTime() );
				User* testUser = nullptr;
				testUser->GetPort();
			}

			Packet sendPacket( Protocol::EventIncreaseResult );
			sendPacket << (std::string)( "PktEventIncreaseHandler result" );
			user->SendPacket( sendPacket );

		}
	);
}
