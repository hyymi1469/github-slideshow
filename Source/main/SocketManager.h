#pragma once
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include "Singleton.h"
#include "Enum.h"


class SocketManager
	:
	public Singleton< SocketManager >
{
private:
	SOCKET*   m_listenSocket;    // 서버의 Listen 소켓 포인터
	WSAEVENT* m_sockEvent;       // 소켓의 이벤트 객체 포인터
	char      m_curListenCount;  // 현재 Listen 하는 개수
	//Acceptor  m_acceptor;        // accept

public:
	// 생성자
	SocketManager();

	// 소멸자
	virtual ~SocketManager();

	// 소켓을 시작한다.
	bool Initialize( const ESocketType socketType );

	// Accept하여 클라이언트 접속시 새로운 소켓을 넘겨주는 메소드
	SOCKET AcceptConnection( DWORD& connectIndex );

private:
	// 리슨을 시작한다.(TCP)
	bool _StartTcpListenSock( const int port );
};
