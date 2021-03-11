#include "ThreadManager.h"
#include "User.h"
#include "../Packet/Packet.h"
#include "../Packet/PktEventIncreaseHandler.h"
#include "../Packet/PktEventDecreaseHandler.h"
#include "../Packet/PktRequestDisconnectHandler.h"
#include "../Packet/PktTestHandler.h"
#include "../Packet/Protocol.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
User::User()
	:
	m_logicThread( nullptr )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소멸자
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
User::~User()
{

}

void User::OnDestroy()
{
	super::OnDestroy();
	Reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	해당 클래스의 멤버들을 리셋한다.
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::Reset()
{
	if ( m_logicThread )
		m_logicThread->DecreaseTaskCount();
	
	m_logicThread = nullptr;
	super::Reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	활동 중인 로직스레드를 설정한다.
/// 
/// @param	logicThread  활동 중인 로직 스레드
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::SetLogicThread( LogicThread* logicThread )
{
	m_logicThread = logicThread;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	활동 중인 로직스레드를 반환한다.
///
/// @return	m_logicThread
////////////////////////////////////////////////////////////////////////////////////////////////////
LogicThread* User::GetLogicThread()
{
	return m_logicThread;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷을 받아서 분해하고 처리한다.
///
/// @param	recvBuffer  받은 패킷
/// @param	recvSize    받은 크기
/// 
/// @return	none
////////////////////////////////////////////////////////////////////////////////////////////////////
int User::PacketHandler( const char* recvBuffer, const int recvSize )
{
	/*
	if ( !m_logicThread )
		m_logicThread = (LogicThread*)( ThreadManager::GetSingleton()->GetIdleThread( "LogicThread" ) );
	*/
	Packet packet;
	packet.CopyToBuffer( recvBuffer, recvSize );

	if ( !packet.IsValidPacket() )
		return -1;

	const int packetSize = packet.GetPacketSize();
	
	time_t curTime = time( nullptr );
	struct tm* localTime = localtime( &curTime );
	printf( "[%04d-%02d-%02d %02d:%02d:%02d] Recv, PacketId : %d , %dbyte \n",
			localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
			localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
			packet.GetProtocolId(), packetSize );

	if ( !m_logicThread )
	{
		OnDestroy();
		return -1;
	}

	m_lastRecvTime = std::time( nullptr );
	User* user = this;
	switch ( (Protocol)( packet.GetProtocolId() ) )
	{
		case Protocol::Test:
			PktTestHandler::OnHandler( packet, user );
			break;
			
		case Protocol::EventIncrease:
			{
				PktEventIncreaseHandler OnHandler;
				OnHandler.OnHandler( packet, user );
			}
			break;

		case Protocol::EventDecrease:
			PktEventDecreaseHandler::OnHandler( packet, user );
			break;

		case Protocol::RequestDisconnect:
			PktRequestDisconnectHandler::OnHandler( packet, user );
			break;
	}
	
	

	return packetSize;
}
