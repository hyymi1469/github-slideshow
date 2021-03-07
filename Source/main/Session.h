#pragma once
#include <ctime>
#include <string>
#include <WinSock2.h>
#include "../Packet/Packet.h"
#include "Defines.h"
#include "Enum.h"


class Session
{
	// 오버레피드의 확장
	typedef struct OVERLAPPEDEX : OVERLAPPED
	{
		ESessionIoFlag flags;
	} OVERLAPPEDEX;

protected:
	SOCKET       m_socket;             // 통신을 위한 소켓
	char*        m_receiveBuffer;      // 받을 데이터의 버퍼(일반 배열은 스택영역이므로 너무 적다. 동적할당해서 힙영역에 만들기)
	int          m_receivedPacketSize; // 받은 데이터의 크기
	std::string  m_ip;                 // ip
	int          m_port;               // port
	const int    m_recvBufferSize;     // 버퍼의 크기
	HANDLE       m_recvEvent;          // Recv 이벤트
	int			 m_sendingPacketSize;  // 현재 send 중인 패킷의 사이즈
	OVERLAPPEDEX m_overlappedRecv;     // 오버레피드 recv확장
	OVERLAPPEDEX m_overlappedSend;     // 오버레피드 send확장
	INT64        m_lastRecvTime;       // 패킷을 마지막으로 받은 시간(초 단위)
	int          m_sessionNum;         // 세션 넘버
	
public:
	// 생성자
	Session();

	// 소멸자
	virtual ~Session();

	// 세션을 폐쇄한다.
	virtual void OnDestroy();

	// 해당 클래스의 멤버들을 리셋한다.
	virtual void Reset();

	// 소켓을 설정한다.
	void SetSocket( SOCKET socket );

	// 세션 연결에 대한 설정을 한다.
	bool OnCreate();

	// recv를 요청한다.(미완성)
	bool Receive();

	// session연결이 되었는지 확인한다.
	bool IsConnected();

	// 패킷을 분해한다.
	void Dispatch( const DWORD bytesTransferred, OVERLAPPED* ov );

	// 받은 패킷을 분해한다.(받은 OV가 recv일 경우 실행)
	void DispatchReceive( const DWORD bytesTransferred );

	// 포트번호를 반환한다.
	int GetPort();

	// 세션넘버를 반환한다.
	int GetSessionNum();

	// 세션넘버를 설정한다.
	void SetSessionNum( const int sessionNum );

	// 패킷을 마지막으로 받은 시간을 반환한다.
	INT64 GetLastRecvTime();

	// 패킷을 마지막으로 받은 시간을 설정한다.
	void SetLastRecvTime( const INT64 lastRecvTime );

	// 세션에 할당된 소켓을 닫음
	void CloseConnection();

	// 패킷을 보낸다.
	void SendPacket( Packet& packet );

	// 패킷을 분해하고 처리한다.
	virtual int PacketHandler( const char* pRecvBuffer, const int nRecvSize ) = 0;
};

