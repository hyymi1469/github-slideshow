#pragma once
#include <string>
#include "Singleton.h"

class Environment
	:
	public Singleton< Environment >

{
private:
	std::string m_gameServerIp;     // 게임서버 아이피
	int         m_gameServerPort;   // 게임서버 포트
	short       m_logicThreadCount; // 룸 스레드 갯수
	short       m_ioThreadCount;    // I/O 스레드 갯수

public:
	// 생성자
	Environment();

	// 소멸자
	virtual ~Environment();

	// 게임서버 아이피를 설정한다.
	void SetGameServerIp( const std::string& gameServerIp );

	// 게임서버 아이피를 반환한다.
	std::string GetGameServerIp();

	// 게임서버 포트를 설정한다.
	void SetGameServerPort( const int gameServerPort );

	// 게임서버 포트를 반환한다.
	int GetGameServerPort() const;

	// 룸 스레드 갯수를 설정한다.
	void SetLogicThreadCount( const short roomThreadCount );

	// 룸 스레드 갯수를 반환한다.
	int GetLogicThreadCount() const;

	// I/O 스레드 갯수를 설정한다.
	void SetIoThreadCount( const short ioThreadCount );

	// I/O 스레드 갯수를 반환한다.
	int GetIoThreadCount() const;
};

