#pragma once
#include "LogicThread.h"
#include "Session.h"


class User
	:
	public Session
{
	typedef Session super;

private:
	LogicThread* m_logicThread; // 현재 해당 유저가 활동 중인 로직 스레드

public:
	// 생성자
	User();

	// 소멸자
	virtual ~User();

	void OnDestroy() override;

	// 해당 클래스의 멤버들을 리셋한다.
	void Reset() override;

	// 활동 중인 로직스레드를 설정한다.
	void SetLogicThread( LogicThread* logicThread );

	// 활동 중인 로직스레드를 반환한다.
	LogicThread* GetLogicThread();

	// 패킷을 받아서 분해하고 처리한다.
	int PacketHandler( const char* pRecvBuffer, const int nRecvSize ) override;
};

