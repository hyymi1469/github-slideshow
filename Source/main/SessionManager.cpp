#include "IocpManager.h"
#include "SessionManager.h"
#include "ThreadManager.h"
#include "User.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
SessionManager::SessionManager()
	:
	m_curMaxSessionNum( 0 )
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소멸자
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
SessionManager::~SessionManager()
{
	Session* session = nullptr;
	while ( m_userObjectPoolQue.pop( session ) )
	{
		delete session;
		session = nullptr;
	}
	
	{
		std::unique_lock< std::mutex > lock( m_mutex );
		for ( auto iter : m_activeObjectPoolMap )
		{
			Session* session = iter.second;
			delete session;
			session = nullptr;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	유저풀을 생성한다.
/// 
/// @return	bool
////////////////////////////////////////////////////////////////////////////////////////////////////
void SessionManager::Initialize()
{
	_PushObjPool( USER_OBJ_POOL );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	새로운 Session 객체를 얻는다.
///
/// @param	socket  연결 중인 소켓
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void SessionManager::CreateNewSession( SOCKET socket )
{
	// Linger off -> 현재 데이터 존재 여부 상관없이 즉시 소켓 닿음
	struct linger lingerOpt;
	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;

	//int timeOutValue = 3000;
	setsockopt( socket, SOL_SOCKET, SO_LINGER, (char*)&lingerOpt, sizeof( lingerOpt ) );
	//setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeOutValue, sizeof( timeOutValue ) );

	int socklen = socklen = BUFFER_SIZE; // 16진수가 CPU가 읽기 쉽고 빠른 수
	setsockopt( socket, SOL_SOCKET, SO_RCVBUF, (char*)&socklen, sizeof( socklen ) );
	setsockopt( socket, SOL_SOCKET, SO_SNDBUF, (char*)&socklen, sizeof( socklen ) );

	User* user = _PopObjPool();
	if ( !user )
	{
		printf( "[SessionManager::CreateNewSession] user == nullptr!\n" );
		return;
	}

	user->Reset();
	user->SetSocket( socket );
	user->SetLastRecvTime( std::time( nullptr ) );
	user->SetLogicThread( (LogicThread*)( ThreadManager::GetSingleton()->GetIdleThread( "LogicThread" ) ) );
	user->GetLogicThread()->IncreaseTaskCount();
	
	if ( !IocpManager::GetSingleton()->Associate( (HANDLE)( socket ), (ULONG_PTR)( user ) ) )
	{
		user->OnDestroy();

		printf("[SessionManager::CreateNewSession] create session fail!\n");

		return;
	}

	if ( !user->OnCreate() )
	{
		user->OnDestroy();

		printf( "[SessionManager::CreateNewSession] OnCreate fail!\n" );

		return;
	}

	if ( !user->IsConnected() )
	{
		user->OnDestroy();

		printf( "[SessionManager::CreateNewSession] IsConnected fail!\n" );

		return;
	}

	{
		std::unique_lock< std::mutex > lock( m_mutex );
		m_activeObjectPoolMap.emplace( user->GetSessionNum(), user );

		time_t curTime = time( nullptr );
		struct tm* localTime = localtime( &curTime );
		printf( "[%04d-%02d-%02d %02d:%02d:%02d] Connect, sessionNum : %d \n",
				localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
				localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
				user->GetSessionNum() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	오브젝트를 반환한다.
///
/// @param	user  반환할 오브젝트
/// 
/// @return	bool
////////////////////////////////////////////////////////////////////////////////////////////////////
void SessionManager::PushObjPool( User* user )
{
	if ( !user )
		return;

	if ( !m_userObjectPoolQue.push( user ) )
	{
		delete user;
		user = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	활동 중인 오브젝트를 뺀다.
///
/// @param	activeSessionNum  활동 중인 세션 넘버
/// 
/// @return	bool
////////////////////////////////////////////////////////////////////////////////////////////////////
void SessionManager::PopActiveObject( const INT64 activeSessionNum )
{
	std::unique_lock< std::mutex > lock( m_mutex );
	m_activeObjectPoolMap.erase( activeSessionNum );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	오브젝트 풀을 넣는다.
///
/// @param	objCount  추가할 오브젝트 카운트
/// 
/// @return	bool
////////////////////////////////////////////////////////////////////////////////////////////////////
void SessionManager::_PushObjPool( const int objCount )
{
	for ( int i = 0; i < objCount; ++i )
	{
		User* user = new User();
		user->SetSessionNum( ++m_curMaxSessionNum );
		if ( !m_userObjectPoolQue.push( user ) )
		{
			delete user;
			user = nullptr;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	유저풀에서 꺼낼 떄 풀이 비어있으면 추가로 200개 풀을 넣는다.
///
/// @param	session  연결할 세션을 넣는다.
/// 
/// @return	bool
////////////////////////////////////////////////////////////////////////////////////////////////////
User* SessionManager::_PopObjPool()
{
	// 만약 pop에 실패한 경우, 풀을 추가로 200 할당한다.
	Session* session = nullptr;
	if ( !m_userObjectPoolQue.pop( session ) )
	{
		printf( "m_userObjectPoolQue count : %d\n", m_userObjectPoolQue.size() );
		session = new User();
		session->SetSessionNum( ++m_curMaxSessionNum );
		m_userObjectPoolQue.push( session );
		_PushObjPool( EXTRA_USER_OBJ_POOL );
	}
	return (User*)( session );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	마지막 패킷 수신 시간을 확인 후 세션을 자른다.
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void SessionManager::CheckAndDisconnect()
{
	Session* session = nullptr;
	const INT64 curTime = std::time( nullptr );
	std::list< Session* > deleteCandidateList;
	
	{
		std::unique_lock< std::mutex > lock( m_mutex );
		for ( auto iter = m_activeObjectPoolMap.begin(); iter != m_activeObjectPoolMap.end(); ++iter )
		{
			Session* session = iter->second;
			if ( !session )
				continue;

			const INT64 lastRecvTime = session->GetLastRecvTime();

			if ( !lastRecvTime )
			{
				deleteCandidateList.emplace_back( session );
				continue;
			}

			const INT64 diffTime = curTime - session->GetLastRecvTime();
			if ( DISCONNECT_SESSION_SEC < diffTime )
			{
				deleteCandidateList.emplace_back( session );
				continue;
			}
		}

		for ( auto iter : deleteCandidateList )
		{
			Session* session = iter;
			if ( !session )
				continue;

			User* user = (User*)( session );
			//user->OnDestroy();
			user->CloseConnection();
			m_activeObjectPoolMap.erase( user->GetSessionNum() );
			PushObjPool( user );
			continue;
		}
	}
	
}
