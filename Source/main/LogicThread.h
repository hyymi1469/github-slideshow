#pragma once
#include <map>
#include "Logic.h"
#include "TaskManager.h"
#include "Thread.h"
#include "ThreadManager.h"


class LogicThread
	:
	public Thread
	//public std::enable_shared_from_this< LogicThread >
{
	typedef Thread super;

private:
	TaskManager        m_taskManager; // 작업 관리자

public:
	// 생성자
	LogicThread();

	// 소멸자
	virtual ~LogicThread();

	// 스레드 생성을 시작한다.
	void Begin() override;

	// 스레드를 작동시킨다.
	void Run() override;

	// 방 쓰레드에서 작업을 실행한다.
	void RunTask( const TaskManager::Task& task );

};


typedef std::shared_ptr< LogicThread > logicThreadPtr;
