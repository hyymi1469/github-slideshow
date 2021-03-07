#include <iostream>
#include <memory>
#include <winsock2.h>
#include "Accepter.h"
#include "Defines.h"
#include "Environment.h"
#include "SocketManager.h"
#include "ThreadManager.h"

#pragma warning(disable : 4996)
#pragma comment(lib, "Ws2_32.lib")


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
SocketManager::SocketManager()
	:m_listenSocket( 0 ), m_sockEvent( 0 ), m_curListenCount( 0 )
{
	// 윈속 초기화
	WSADATA wsa;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 )
	{
		printf( "SocketManager::SocketManager WSAStartup error\n" );
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소멸자
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
SocketManager::~SocketManager()
{
	delete m_listenSocket;
	m_listenSocket = nullptr;

	delete m_sockEvent;
	m_sockEvent = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소켓을 설정하고 리슨할 갯수 및 이벤트를 설정한다.
///
/// @param	socketType  소켓 타입
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SocketManager::Initialize( const ESocketType socketType )
{
	if ( MAX_LISTEN_COUNT > WSA_MAXIMUM_WAIT_EVENTS )
		return false;

	if ( !m_listenSocket )
		m_listenSocket = new SOCKET[ MAX_LISTEN_COUNT ];

	if ( !m_sockEvent )
		m_sockEvent = new WSAEVENT[ MAX_LISTEN_COUNT ];

	for ( int i = 0; i != MAX_LISTEN_COUNT; ++i )
	{
		m_listenSocket[ i ] = INVALID_SOCKET;
		m_sockEvent[ i ] = INVALID_HANDLE_VALUE;
	}

	if ( socketType == ESocketType::Tcp )
	{
		// 리슨을 실행한다.(enum class EAcceptType 의 순서에 따라 _StartTcpListenSock 호출 순서를 유지해야 한다.)
		if ( !_StartTcpListenSock( Environment::GetSingleton()->GetGameServerPort() ) )
			return false;


		if ( !ThreadManager::GetSingleton()->CreateThread( "Accepter", new Accepter ) )
		{
			printf( "[SocketManager::Initialize] CreateThread failed!" );
			return false;
		}		
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	리슨을 시작한다.
///
/// @param	port  포트 번호
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SocketManager::_StartTcpListenSock( const int port )
{
	int retval = 0;
	SOCKET socket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED );
	if ( socket == INVALID_SOCKET )
	{
		::WSACleanup();
		return false;
	}

	// 보통 소켓을 사용하는 프로그램은 강제 종료되었지만 커널단에서 해당 소켓을 바인딩해서 사용하고 있기 때문에 발생하는 에러를 위해 reuse옵션을 켬
	bool reuse = true;
	setsockopt( socket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof( reuse ) );
	
	// Event 생성과 설정 ( 클라이언트 연결 )
	WSAEVENT listenEvent = WSACreateEvent();
	WSAEventSelect( socket, listenEvent, FD_ACCEPT );

	// Bind
	SOCKADDR_IN serverSockAddr;
	memset( &serverSockAddr, 0, sizeof( serverSockAddr ) );
	serverSockAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	serverSockAddr.sin_family = AF_INET;
	serverSockAddr.sin_port = htons( port );
	retval = bind( socket, (LPSOCKADDR)( &serverSockAddr ), sizeof( serverSockAddr ) );
	if ( retval == SOCKET_ERROR )
	{
		printf( "Bind Error\n" );
		closesocket( socket );
		WSACleanup();
		return false;
	}

	//Listen
	retval = listen( socket, SOMAXCONN );
	if ( retval == SOCKET_ERROR )
	{
		closesocket( socket );
		socket = INVALID_SOCKET;
		WSACleanup();
		return false;
	}
	
	m_listenSocket[ m_curListenCount ] = socket;
	m_sockEvent[ m_curListenCount ] = listenEvent;
	++m_curListenCount;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Accept하여 클라이언트 접속시 새로운 소켓을 넘겨주는 메소드
/// 
/// @param	connectIndex  enum class EAcceptType를 가져온다.
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
SOCKET SocketManager::AcceptConnection( DWORD& connectIndex )
{
	// Event가 발생할 때까지 기다림
	DWORD dwIndex = WSAWaitForMultipleEvents( m_curListenCount, m_sockEvent, FALSE, INFINITE, FALSE );
	connectIndex = dwIndex - WSA_WAIT_EVENT_0;

	WSAResetEvent( m_sockEvent[ connectIndex ] );

	// Network 상에서 발생한 Event를 찾음
	WSANETWORKEVENTS networkEvents;
	WSAEnumNetworkEvents( m_listenSocket[ connectIndex ], m_sockEvent[ connectIndex ], &networkEvents );

	if ( networkEvents.lNetworkEvents & FD_ACCEPT )
		return accept( m_listenSocket[ connectIndex ], NULL, NULL );
	
	return INVALID_SOCKET;
}
