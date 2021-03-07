#pragma once
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <cstdio>
#include "LogicThread.h"
#include "Singleton.h"
#include "Thread.h"


typedef std::set< std::string >                   ThreadNameSet;     // 스레드 이름이 정의되어 있는 셋
typedef std::vector< HANDLE >                     ThreadPoolVector;  // 스레드풀 벡터
typedef std::map< std::string, ThreadPoolVector > ThreadMap;         // 스레드 이름, 스레드 디테일

class ThreadManager
	:
	public Singleton< ThreadManager >
{
private:
	ThreadNameSet   m_threadNameSet;  // 스레드 이름 목록 셋
	ThreadMap       m_threadMap;      // 스레드 관리 맵
	std::mutex      m_mutex;          // 뮤텍스

public:
	// 생성자
	ThreadManager();

	// 소멸자
	virtual ~ThreadManager();

	// 스레드를 생성한다.
	bool CreateThread( const std::string& threadName, Thread* threadPtr );
	
	// 스레드를 생성한다.( beginthreadexVer)
	HANDLE BeginCreateThread(
		LPTHREAD_START_ROUTINE startAddress,
		LPVOID                 param,
		UINT&                  threadId );

	// 가장 일이 없는 스레드를 반환한다.
	Thread* GetIdleThread( const std::string& threadStr );
	/*
	// 테스트
	ThreadPoolVector GetThreadPool( const std::string& threadStr );

	void AcquireWaitingRoom();
	*/
};
