#pragma once
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <winsock2.h>


typedef unsigned( __stdcall* THREAD_START )( void* );

class Thread
{
protected:
	bool               m_isStarted;     // 스레드 시작여부
	unsigned int       m_threadId;      // 스레드의 ID값
	HANDLE	           m_threadHandle;  // 스레드의 핸들값
	std::atomic< int > m_taskCount;     // 작업 갯수 

public:
	// 생성자
	Thread();

	// 소멸자
	virtual ~Thread();

	// 스레드 생성을 준비한다.(_beginthreadex Ver)
	void BeginThread();

	// 스레드 생성한다.(_beginthreadex Ver)
	static DWORD HandleRunner( LPVOID param );

	// 스레드를 작동시킨다.
	virtual void Run() = 0;

	// 스레드 생성을 시작한다.
	virtual void Begin() = 0;

	// 종료시 처리사항
	void End();

	// logic갯수를 반환한다.
	int GetTaskCount();

	// logic갯수를 증가시킨다.
	void IncreaseTaskCount();

	// logic갯수를 감소시킨다.
	void DecreaseTaskCount();

	// 스레드 식별자를 반환한다.
	unsigned int GetThreadId();
};


typedef std::shared_ptr< Thread > ThreadPtr;
