#pragma warning(disable : 4996)
#pragma warning( disable : 4789 )

#include <memory>
#include "Session.h"
#include "SessionManager.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
Session::Session()
	:
	m_recvEvent( WSACreateEvent() ),
	m_recvBufferSize( BUFFER_SIZE ),
	m_sendingPacketSize( 0 ),
	m_receiveBuffer( nullptr ),
	m_lastRecvTime( 0 ),
	m_sessionNum( 0 ),
	m_port( 0 ),
	m_receivedPacketSize( 0 ),
	m_socket( INVALID_SOCKET )
{
	
	if ( !m_receiveBuffer )
	{
		m_receiveBuffer = new char[ BUFFER_SIZE ];
		memset( m_receiveBuffer, 0, BUFFER_SIZE );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소멸자
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
Session::~Session()
{
	if ( m_recvEvent != INVALID_HANDLE_VALUE )
	{
		WSACloseEvent( m_recvEvent );

		m_recvEvent = INVALID_HANDLE_VALUE;
	}

	OnDestroy();

	if ( m_receiveBuffer )
	{
		delete m_receiveBuffer;
		m_receiveBuffer = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	세션을 폐쇄한다.
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::OnDestroy()
{
	time_t curTime = time( nullptr );
	struct tm* localTime = localtime( &curTime );
	printf( "[%04d-%02d-%02d %02d:%02d:%02d] [Session::OnDestroy] disconnect, sessionNum : %d \n",
			localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
			localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
			m_sessionNum );

	CloseConnection();
	
	SessionManager::GetSingleton()->PushObjPool( (User*)( this ) );
	SessionManager::GetSingleton()->PopActiveObject( m_sessionNum );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	해당 클래스의 멤버들을 리셋한다.
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::Reset()
{
	m_socket = INVALID_SOCKET;
	//m_receiveBuffer = nullptr;
	m_receivedPacketSize = 0;
	m_ip = "";
	m_port = 0;
	//m_recvBufferSize = 0;
	//m_recvEvent = INVALID_HANDLE_VALUE;
	m_sendingPacketSize = 0;
	m_lastRecvTime = 0;
	//m_sessionNum = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소켓을 설정한다.
///
/// @param	socket  연결 중인 소켓
/// 
/// @return	bool
////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::SetSocket( SOCKET socket )
{
	m_socket = socket;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	세션 연결에 대한 설정을 한다.
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Session::OnCreate()
{
	m_receivedPacketSize = 0;

	if ( m_socket == INVALID_SOCKET )
		return false;

	SOCKADDR_IN sockAddr;
	int size = sizeof( sockAddr );
	if ( getpeername( m_socket, (SOCKADDR*)( &sockAddr ), &size ) != SOCKET_ERROR )
	{
		m_port = sockAddr.sin_port;
		m_ip = inet_ntoa( sockAddr.sin_addr );
	}
	else
	{
		printf("[Session::OnCreate], getpeername fail!\n");
		return false;
	}

	Receive();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	recv를 요청한다.(미완)
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Session::Receive()
{
	if ( m_socket == INVALID_SOCKET )
		printf("[Session::Receive] m_socket == nullptr\n");

	DWORD	recvBytes = 0;
	DWORD	flags     = 0;
	DWORD   bufSize   = m_recvBufferSize - m_receivedPacketSize;
	int		retval    = 0;
	WSABUF wsaBuffer;

	::ZeroMemory( &m_overlappedRecv, sizeof( OVERLAPPEDEX ) );
	m_overlappedRecv.flags = ESessionIoFlag::AsyncFlagReceive;

	// WSABUF 구조체 셋팅
	wsaBuffer.buf = m_receiveBuffer + m_receivedPacketSize;
	wsaBuffer.len = bufSize;

	retval = WSARecv( 
					m_socket,
					&wsaBuffer,
					1,
					&recvBytes,
					&flags,
					(OVERLAPPEDEX*)( &m_overlappedRecv ),
					nullptr );

	if ( retval == SOCKET_ERROR )
	{
		int errorCode = WSAGetLastError();
		if ( errorCode != ERROR_IO_PENDING )
		{
			if ( errorCode == WSAECONNRESET || errorCode == WSAENETRESET || errorCode == WSAENOTCONN || errorCode == WSAECONNABORTED )
			{
				time_t curTime = time( nullptr );
				struct tm* localTime = localtime( &curTime );
				printf( "[%04d-%02d-%02d %02d:%02d:%02d] [Session::Receive] SessionId : %d \n",
						localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
						localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
						m_sessionNum );
				OnDestroy();
				//ForcedDisconnect( errorCode );
			}

			//printf("WSAError! code : %d", errorCode );
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	session연결이 되었는지 확인한다.
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Session::IsConnected()
{
	if ( m_recvEvent == INVALID_HANDLE_VALUE || m_socket == INVALID_SOCKET )
		return false;

	if ( !m_sessionNum )
	{
		printf("m_sessionNum == 0\n");
		return false;
	}

	WSANETWORKEVENTS netEvent;
	bool retval = false;

	::ZeroMemory( &netEvent, sizeof( netEvent ) );
	::WSAEventSelect( m_socket, m_recvEvent, FD_READ | FD_CLOSE );
	::WSAEnumNetworkEvents( m_socket, m_recvEvent, &netEvent );

	retval = ( ( netEvent.lNetworkEvents & FD_CLOSE ) == FD_CLOSE );

	return !( retval );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷을 분해한다.
///
/// @param	bytesTransferred  받은 크기
/// @param	ov                OVERLAPPED
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::Dispatch( const DWORD bytesTransferred, OVERLAPPED* ov )
{
	if ( !bytesTransferred )
		return;

	OVERLAPPEDEX* overlapped = (OVERLAPPEDEX*)( ov );
	
	switch ( overlapped->flags )
	{
		case ESessionIoFlag::AsyncFlagSend:
		m_sendingPacketSize -= bytesTransferred;
		break;

		case ESessionIoFlag::AsyncFlagReceive:
		DispatchReceive( bytesTransferred );
		break;
	}
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	받은 패킷을 분해한다.(받은 OV가 recv일 경우 실행)
///
/// @param	bytesTransferred  받은 크기
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::DispatchReceive( const DWORD bytesTransferred )
{
	// 지금까지 받은 패킷양
	m_receivedPacketSize += bytesTransferred;

	char* readPivot = m_receiveBuffer;

	while ( m_receivedPacketSize > 0 )
	{
		// PacketHandler() 은 packet 을 파싱 후 패킷 크기를 리턴한다.
		int packetSize = PacketHandler( readPivot, m_receivedPacketSize );

		if ( packetSize > 0 )
		{
			m_receivedPacketSize -= packetSize;
			readPivot += packetSize;
		}
	}

	if ( m_receivedPacketSize > 0 )
	{
		::memmove( m_receiveBuffer, readPivot, m_receivedPacketSize );
	}

	Receive();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	포트번호를 반환한다.
/// 
/// @return	m_lastRecvTime
////////////////////////////////////////////////////////////////////////////////////////////////////
int Session::GetPort()
{
	return m_port;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	세션넘버를 반환한다.
/// 
/// @return	m_sessionNum
////////////////////////////////////////////////////////////////////////////////////////////////////
int Session::GetSessionNum()
{
	return m_sessionNum;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	세션넘버를 설정한다.
/// 
/// @param	sessionNum  세션넘버
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::SetSessionNum( const int sessionNum )
{
	m_sessionNum = sessionNum;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷을 마지막으로 받은 시간을 반환한다.
/// 
/// @return	m_lastRecvTime
////////////////////////////////////////////////////////////////////////////////////////////////////
INT64 Session::GetLastRecvTime()
{
	return m_lastRecvTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷을 마지막으로 받은 시간을 반환한다.
/// 
/// @param	lastRecvTime  패킷을 마지막으로 받은 시간
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::SetLastRecvTime( const INT64 lastRecvTime )
{
	m_lastRecvTime = lastRecvTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	세션에 할당된 소켓을 닫음
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::CloseConnection()
{
	if ( m_socket == INVALID_SOCKET )
		return;

	shutdown( m_socket, SD_BOTH );
	closesocket( m_socket );
	m_socket = INVALID_SOCKET;
	Reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷을 보낸다.
/// 
/// @param	packet  보낼 패킷
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::SendPacket( Packet& packet )
{
	if ( m_socket == INVALID_SOCKET )
		OnDestroy();

	DWORD	sentBytes = 0;
	DWORD	flags     = 0;
	int		retval    = 0;
	WSABUF  wsaBuffer;
	char*   sendPacket = packet.GetPacketBuffer();
	int     size =       packet.GetPacketSize();

	::ZeroMemory( &m_overlappedSend, sizeof( OVERLAPPEDEX ) );
	m_overlappedSend.flags = ESessionIoFlag::AsyncFlagSend;

	wsaBuffer.buf = sendPacket;
	wsaBuffer.len = size;

	retval = WSASend(
					m_socket,
					&wsaBuffer,
					1,
					&sentBytes,
					flags,
					(OVERLAPPEDEX*)&m_overlappedSend,
					NULL );

	if ( SOCKET_ERROR == retval )
	{
		int nErrorCode = WSAGetLastError();
		if ( nErrorCode != ERROR_IO_PENDING )
		{
			if ( nErrorCode == WSAECONNRESET || nErrorCode == WSAENETRESET || nErrorCode == WSAENOTCONN || nErrorCode == WSAECONNABORTED )
			{
				//OutputLog( g_phoenixlog, "[SEND_SOCKET_ERROR] Errorcode : %d |IP:%s|", nErrorCode, m_szIP );
				//ForcedDisconnect( nErrorCode );
			}

			OnDestroy();
		}
	}

	m_sendingPacketSize += sentBytes;

	if ( m_sendingPacketSize > BUFFER_SIZE )
	{
		//OutputLog( g_phoenixlog, "[SENDINGPACKETSIZE_FULL] SendingSize : %d", m_nSendingPacketSize );
		OnDestroy();
		//ForcedDisconnect( SENDINGPACKETSIZE_FULL );
	}

	time_t curTime = time( nullptr );
	struct tm* localTime = localtime( &curTime );
	printf( "[%04d-%02d-%02d %02d:%02d:%02d] Send, PacketId : %d , %dbyte \n",
			localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
			localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
			packet.GetProtocolId(), packet.GetPacketSize() );
}