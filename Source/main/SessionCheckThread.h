#pragma once
#include "SessionManager.h"
#include "Thread.h"
#include "ThreadManager.h"


class SessionCheckThread
	:
	public Thread
	//public std::enable_shared_from_this< LogicThread >
{
	typedef Thread super;

public:
	// 생성자
	SessionCheckThread();

	// 소멸자
	virtual ~SessionCheckThread();

	// 스레드 생성을 시작한다.
	void Begin() override;

	// 스레드를 작동시킨다.
	void Run() override;
};


typedef std::shared_ptr< SessionCheckThread > SessionCheckThreadPtr;
