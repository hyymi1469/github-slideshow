#include <thread>
#include "IocpManager.h"
#include "IoThread.h"
#include "Logic.h"
#include "SessionManager.h"
#include "User.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
IoThread::IoThread()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소멸자
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
IoThread::~IoThread()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스레드 생성을 시작한다.
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void IoThread::Begin()
{
	if ( m_isStarted )
		return;

	m_isStarted = true;

	std::thread myThread = std::thread( &IoThread::Run, this );
	myThread.detach();

	std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스레드를 작동시킨다.
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void IoThread::Run()
{
	std::stringstream ss;
	ss << std::this_thread::get_id();
	m_threadId = atoi( ss.str().c_str() );
	printf( "IoThread start : %d\n", m_threadId );

	bool retval = 0;
	HANDLE handleIocp = IocpManager::GetSingleton()->GetWorkerIOCPHandle();
	DWORD bytesTransfer = 0;
	ULONG_PTR completionKey = 0;
	LPOVERLAPPED overlapped = 0;
	User* user = nullptr;

	// lockfree test용
	//std::vector< HANDLE > vec = ThreadManager::GetSingleton()->GetThreadPool( "LogicThread" );
	while ( true )
	{
		// 우선 이걸로 해서 맵 제대로 도는지 확인후 스레드 여러 개로 불러와서 다시 해보기!
		/*
		for ( auto iter : vec )
		{
			LogicThread* thread = (LogicThread*)( iter );
			auto threadId = thread->GetThreadId();

			thread->RunTask(
				[threadId] ()
				{
					if ( Logic::GetSingleton()->m_logicLIst.size() < 5000 )
					{
						Logic::GetSingleton()->m_logicLIst.emplace_back( 1 );
						printf( "m_testMap size : %d : %d\n", Logic::GetSingleton()->m_logicLIst.size(), threadId );
					}
				} );
		}
		*/
		
		retval = ::GetQueuedCompletionStatus( handleIocp, &bytesTransfer, &completionKey, &overlapped, INFINITE );
		if ( overlapped == nullptr )
			continue;

		user = (User*)( completionKey );
		if ( !user )
			continue;

		if ( !retval )
		{
			time_t curTime = time( nullptr );
			struct tm* localTime = localtime( &curTime );
			printf( "[%04d-%02d-%02d %02d:%02d:%02d] disconnected, sessionNum : %d \n",
					localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
					localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
					user->GetSessionNum() );
			user->OnDestroy();
			
			continue;
		}

		if ( retval && bytesTransfer == 0 )
		{
			user->OnDestroy();
			continue;
		}

		user->Dispatch( bytesTransfer, overlapped );

	}

}
