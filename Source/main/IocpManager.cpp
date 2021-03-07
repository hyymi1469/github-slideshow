#include <memory>
#include <vector>
#include "Environment.h"
#include "IocpManager.h"
#include "IoThread.h"
#include "LogicThread.h"
#include "SessionCheckThread.h"
#include "ThreadManager.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
///
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
IocpManager::IocpManager()
	:m_iocpHandle( nullptr )
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소멸자
///
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
IocpManager::~IocpManager()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	IOCP 핸들, Worker Thread Pool 생성하는 메서드
///
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IocpManager::Initialize()
{
	m_iocpHandle = CreateIoCompletionPort( INVALID_HANDLE_VALUE, nullptr, 0, 0 );
	if ( !m_iocpHandle )
		return false;

	// 스레드를 만들기 위해 shared 할당
	std::vector< LogicThread* > logicThreadVec;
	for ( int i = 0; i < Environment::GetSingleton()->GetLogicThreadCount(); ++i )
		logicThreadVec.emplace_back( new LogicThread() );
	
	// Logic(Worker)스레드 생성
	for ( auto iter : logicThreadVec )
	{
		if ( !ThreadManager::GetSingleton()->CreateThread( "LogicThread", iter ) )
		{
			printf("[IocpManager::Initialize] CreateThread fail!\n");
			return false;
		}
	}


	// I/O스레드 생성
	std::vector< IoThread* > ioThreadVec;
	for ( int i = 0; i < Environment::GetSingleton()->GetIoThreadCount(); ++i )
		ioThreadVec.emplace_back( new IoThread() );

	for ( auto iter : ioThreadVec )
	{
		if ( !ThreadManager::GetSingleton()->CreateThread( "IoThread", iter ) )
		{
			printf( "[IocpManager::Initialize] CreateThread fail!\n" );
			return false;
		}
	}

	// 일정 시간마다 상태 확인
	SessionCheckThread* sessionCheckThread = new SessionCheckThread();
	if ( !ThreadManager::GetSingleton()->CreateThread( "SessionCheckThread", sessionCheckThread ) )
	{
		printf( "[IocpManager::Initialize] CreateThread fail!\n" );
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	IOCP 핸들과 소켓 핸들을 연결한다.
///
/// @param	socketType     소켓 타입
/// @param	completionKey  실행할 포인터 위치
/// 
/// @return	bool
////////////////////////////////////////////////////////////////////////////////////////////////////
bool IocpManager::Associate( HANDLE device, ULONG_PTR completionKey )
{
	HANDLE retHandle;

	retHandle = CreateIoCompletionPort( device, m_iocpHandle, completionKey, 0 );
	if ( retHandle != m_iocpHandle )
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	m_iocpHandle를 반환한다.
/// 
/// @return	m_iocpHandle
////////////////////////////////////////////////////////////////////////////////////////////////////
HANDLE IocpManager::GetWorkerIOCPHandle()
{
	return m_iocpHandle;
}
