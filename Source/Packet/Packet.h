#pragma once
#pragma warning( disable:4786 )
#pragma warning( disable:4996 )


#include <string>
#include <Windows.h>
#include "../main/Defines.h"
#include "Protocol.h"

class Packet
{
public:
	// 생성자
	Packet();

	// 생성자
	Packet( Protocol protocolId );

	// 생성자
	Packet( const Packet& sourcePacket );

	// 소멸자
	virtual ~Packet();


protected:
	typedef struct HEADER
	{
		char*           startCharacter;
		unsigned short* dataSize;
		int*            protocolId;
	} HEADER;

	HEADER       m_packetHeader;                   // 패킷 해더
	char         m_arrPacketBuffer[ BUFFER_SIZE ]; // 패킷 버퍼
	char*        m_dataField;                      // 데이터가 저장되는 시작 지점 포인터
	char*        m_readPosition;                   // 데이터의 읽는 위치
	char*        m_writePosition;                  // 데이터의 쓰는 위치
	char*        m_endOfDataField;                 // 데이터가 저장되는 마지막 지점 포인터
	unsigned int m_receivedSize;                   // 받은 데이터의 양

public:
	// 패킷이 유효한지 확인한다
	bool IsValidPacket();

	// 프로토콜 넘버를 설정한다.
	Packet& SetProtocolId( const Protocol protocolId );

	// 프로토콜 넘버를 반환한다.
	int GetProtocolId();
	
	// 헤더를 제외한 데이터 사이즈를 반환한다.
	unsigned short GetDataFieldSize();

	// 패킷 총 크기를 반환한다.
	int GetPacketSize();

	// 받은 패킷 크기를 반환한다.
	int	GetReceivedSize();

	// 패킷을 초기화한다.
	void Clear( int bufferSize = BUFFER_SIZE );

	// 현재 패킷 버퍼를 반환한다.
	char* GetPacketBuffer();

	// Buffer 로 받은 데이터를 저장한다.
	void CopyToBuffer( const char* buff, const int size );

	// 패킷에 있는 데이터 내용을 읽는다.
	void ReadData( void* buffer, int size );

	// 패킷에 있는 데이터 내용을 읽는다.
	void ReadData( std::string& strDestination, int size );

	// 패킷 버퍼에 데이터를 쓴다.
	void WriteData( const void* buffer, int size );

	// Packet 클래스의 읽기 위치 포인터를 패킷 버퍼의 실제 데이터의 초기 위치로 이동 시킨다.
	void ResetReadPt();

	// = 연산자 오퍼레이터
	Packet& operator = ( Packet& packet );

	// 연산자 오퍼레이터
	Packet& operator << ( bool source );
	Packet& operator >> ( bool& destination );

	Packet& operator << ( char source );
	Packet& operator >> ( char& destination );

	Packet& operator << ( unsigned short source );
	Packet& operator >> ( unsigned short& destination );

	Packet& operator << ( unsigned int source );
	Packet& operator >> ( unsigned int& destination );

	Packet& operator << ( short source );
	Packet& operator >> ( short& destination );

	Packet& operator << ( int source );
	Packet& operator >> ( int& destination );

	Packet& operator << ( float source );
	Packet& operator >> ( float& destination );

	Packet& operator << ( const std::string& source );
	Packet& operator >> ( std::string& destination );
};
