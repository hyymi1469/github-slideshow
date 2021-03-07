#include "SessionCheckThread.h"

SessionCheckThread::SessionCheckThread()
{
}

SessionCheckThread::~SessionCheckThread()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스레드 생성을 시작한다.
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void SessionCheckThread::Begin()
{
	if ( m_isStarted )
		return;

	m_isStarted = true;

	std::thread myThread = std::thread( &SessionCheckThread::Run, this );
	myThread.detach();

	std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스레드를 작동시킨다.
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void SessionCheckThread::Run()
{
	std::stringstream ss;
	ss << std::this_thread::get_id();
	m_threadId = atoi( ss.str().c_str() );
	printf( "SessionCheckThread start : %d\n", m_threadId );

	while ( true )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 60000 ) );
		time_t curTime = time( nullptr );
		struct tm* localTime = localtime( &curTime );
		printf( "[%04d-%02d-%02d %02d:%02d:%02d] [SessionCheckThread::Run()] SessionCheck \n",
				localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
				localTime->tm_hour, localTime->tm_min, localTime->tm_sec );
		
		SessionManager::GetSingleton()->CheckAndDisconnect();
	}

}