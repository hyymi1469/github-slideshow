#include "Packet.h"
#include <stdio.h>
#include <assert.h>


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::Packet()
	: m_dataField( nullptr ), m_readPosition( nullptr ), m_writePosition( nullptr ), m_receivedSize( 0 )
{
	Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::Packet( Protocol protocolId )
	: m_dataField( nullptr ), m_readPosition( nullptr ), m_writePosition( nullptr ), m_receivedSize( 0 )
{
	Clear();
	SetProtocolId( protocolId );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::Packet( const Packet& sourcePacket )
	: m_dataField( nullptr ), m_readPosition( nullptr ), m_writePosition( nullptr ), m_receivedSize( 0 )
{
	int offset = 0;

	Clear();

	::CopyMemory( m_arrPacketBuffer, sourcePacket.m_arrPacketBuffer, BUFFER_SIZE );
	assert( m_readPosition <= m_endOfDataField && "ReadPosition이 DataField를 넘어갔습니다." );

	// 읽는 위치 설정
	offset = (int)( sourcePacket.m_readPosition - sourcePacket.m_dataField );
	m_readPosition += offset;
	assert( m_readPosition <= m_endOfDataField && "ReadPosition이 DataField를 넘어갔습니다." );

	// 쓰는 위치 설정
	offset = (int)( sourcePacket.m_writePosition - sourcePacket.m_dataField );
	m_writePosition += offset;
	assert( m_writePosition <= m_endOfDataField && "WritePosition이 DataField를 넘어갔습니다." );

	m_receivedSize = sourcePacket.m_receivedSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소멸자
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::~Packet()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷이 유효한지 확인한다
/// 
/// @return	패킷 유효 여부
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Packet::IsValidPacket()
{
	if ( m_receivedSize < PACKET_HEADERSIZE )
		return false;

	if ( m_receivedSize < PACKET_HEADERSIZE + GetDataFieldSize() )
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	프로토콜 넘버를 설정한다.
///
/// @param	protocolId  프로토콜 식별자
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
Packet& Packet::SetProtocolId( const Protocol protocolId )
{
	*m_packetHeader.protocolId = (int)( protocolId );
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	프로토콜 넘버를 반환한다.
/// 
/// @return	프로토콜 식별자
////////////////////////////////////////////////////////////////////////////////////////////////////
int Packet::GetProtocolId()
{
	return *m_packetHeader.protocolId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	헤더를 제외한 데이터 사이즈를 반환한다.
/// 
/// @return	헤더를 제외한 데이터 사이즈
////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned short Packet::GetDataFieldSize()
{
	return *m_packetHeader.dataSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷 총 크기를 반환한다.
/// 
/// @return	패킷 크기
////////////////////////////////////////////////////////////////////////////////////////////////////
int Packet::GetPacketSize()
{
	return ( GetDataFieldSize() + PACKET_HEADERSIZE );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	받은 패킷 크기를 반환한다.
/// 
/// @return	받은 패킷 크기
////////////////////////////////////////////////////////////////////////////////////////////////////
int	Packet::GetReceivedSize()
{
	return m_receivedSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷을 초기화한다.
///
/// @param	bufferSize  버퍼 사이즈
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void Packet::Clear( int bufferSize )
{
	memset( m_arrPacketBuffer, 0, bufferSize );

	// 패킷 헤더 초기화
	m_packetHeader.startCharacter = (char*)m_arrPacketBuffer;

	m_packetHeader.dataSize = (unsigned short*)( m_arrPacketBuffer + PACKET_START_POS );
	m_packetHeader.protocolId = (int*)( m_arrPacketBuffer + PACKET_HEADER_READ );

	m_dataField = &m_arrPacketBuffer[ PACKET_HEADERSIZE ];

	// 데이터의 위치 포인터 초기화
	m_readPosition = m_dataField;
	m_writePosition = m_dataField;
	m_endOfDataField = &m_arrPacketBuffer[ bufferSize ];

	assert( m_readPosition <= m_endOfDataField && "ReadPosition이 DataField를 넘어갔습니다." );

	m_receivedSize = PACKET_HEADERSIZE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	현재 패킷 버퍼를 반환한다.
/// 
/// @return	현재 패킷 버퍼
////////////////////////////////////////////////////////////////////////////////////////////////////
char* Packet::GetPacketBuffer()
{
	return (char*)( m_arrPacketBuffer );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Buffer 로 받은 데이터를 저장한다.
///
/// @param	buff  버퍼
/// @param	size  크기
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void Packet::CopyToBuffer( const char* buff, const int size )
{
	Clear();

	m_receivedSize = 0;

	::CopyMemory( m_arrPacketBuffer + m_receivedSize, buff, size );
	m_receivedSize += size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷에 있는 데이터 내용을 읽는다.
///
/// @param	buff  버퍼
/// @param	size  크기
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void Packet::ReadData( void* buffer, int size )
{
	if ( m_readPosition + size > m_dataField + GetDataFieldSize() ||
		 m_readPosition + size > m_endOfDataField )
	{
		throw( 1 );
	}

	::CopyMemory( buffer, m_readPosition, size );
	m_readPosition += size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷에 있는 데이터 내용을 읽는다.
///
/// @param	strDestination  읽을 string
/// @param	size            크기
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void Packet::ReadData( std::string& strDestination, int size )
{
	if ( m_readPosition + size > m_dataField + GetDataFieldSize() ||
		 m_readPosition + size > m_endOfDataField )
	{
		printf("[Packet::ReadData]Wrong Read Date\n");
		throw( 1 );
	}

	strDestination = m_readPosition;
	m_readPosition += size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷 버퍼에 데이터를 쓴다.
///
/// @param	buffer  쓸 버퍼
/// @param	size    크기
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void Packet::WriteData( const void* buffer, int size )
{
	if ( m_writePosition + size > m_endOfDataField )
	{
		printf("[Packet::WriteData]Packet WriteData exception protocol number = [ % d ]\n", GetProtocolId() );
#ifdef _DEBUG
		assert( 0 && "WritePacket SizeOver!!!!!!!!!!!!!!!!!!!!\n" );
#endif
		return;
	}

	::CopyMemory( m_writePosition, buffer, size );
	m_writePosition += size;
	m_receivedSize += size;

	// 패킷의 데이터 크기 
	*m_packetHeader.dataSize += size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Packet 클래스의 읽기 위치 포인터를 패킷 버퍼의 실제 데이터의 초기 위치로 이동 시킨다.
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void Packet::ResetReadPt()
{
	m_readPosition = m_dataField;
}

// = 연산자 오퍼레이터
Packet& Packet::operator = ( Packet& packet )
{
	if ( this == &packet )
		return *this;

	::CopyMemory( m_arrPacketBuffer, packet.GetPacketBuffer(), packet.GetPacketSize() );

	m_packetHeader.startCharacter = (char*)m_arrPacketBuffer;
	m_packetHeader.dataSize       = (unsigned short*)( m_arrPacketBuffer + PACKET_START_POS );
	m_packetHeader.protocolId     = (int*)( m_arrPacketBuffer + PACKET_HEADER_READ );

	m_dataField = &m_arrPacketBuffer[ 4 + 2 ];

	// 데이터의 위치 포인터 초기화
	m_readPosition = m_dataField;
	m_writePosition = m_dataField;
	m_endOfDataField = &m_arrPacketBuffer[ BUFFER_SIZE ];

	assert( m_readPosition <= m_endOfDataField && "ReadPosition이 DataField를 넘어갔습니다." );

	m_receivedSize = packet.m_receivedSize;

	return *this;
}

Packet& Packet::operator << ( bool source )
{
	WriteData( &source, sizeof( bool ) );

	return *this;
}

Packet& Packet::operator >> ( bool& destination )
{
	ReadData( &destination, sizeof( bool ) );

	return *this;
}

Packet& Packet::operator << ( char source )
{
	WriteData( &source, sizeof( char ) );

	return *this;
}

Packet& Packet::operator >> ( char& destination )
{
	ReadData( &destination, sizeof( char ) );

	return *this;
}

Packet& Packet::operator << ( unsigned short source )
{
	WriteData( &source, sizeof( unsigned short ) );

	return *this;
}

Packet& Packet::operator >> ( unsigned short& destination )
{
	ReadData( &destination, sizeof( unsigned short ) );

	return *this;
}


Packet& Packet::operator << ( unsigned int source )
{
	WriteData( &source, sizeof( unsigned int ) );

	return *this;
}

Packet& Packet::operator >> ( unsigned int& destination )
{
	ReadData( &destination, sizeof( unsigned int ) );

	return *this;
}

Packet& Packet::operator << ( short source )
{
	WriteData( &source, sizeof( short ) );

	return *this;
}

Packet& Packet::operator >> ( short& destination )
{
	ReadData( &destination, sizeof( short ) );

	return *this;
}


Packet& Packet::operator << ( int source )
{
	WriteData( &source, sizeof( int ) );

	return *this;
}

Packet& Packet::operator >> ( int& destination )
{
	ReadData( &destination, sizeof( int ) );

	return *this;
}

Packet& Packet::operator << ( float source )
{
	WriteData( &source, sizeof( float ) );

	return *this;
}

Packet& Packet::operator >> ( float& destination )
{
	ReadData( &destination, sizeof( float ) );

	return *this;
}

Packet& Packet::operator << ( const std::string& source )
{
	WriteData( (LPCTSTR)( source.c_str() ), (int)( source.size() ) + 1 );

	return *this;
}

Packet& Packet::operator >> ( std::string& destination )
{
	int strLength = strlen( m_readPosition ) + 1;

	ReadData( destination, strLength );

	return *this;
}
