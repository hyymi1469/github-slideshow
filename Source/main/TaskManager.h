#pragma once
#include <boost/utility.hpp>
#include <condition_variable>
#include <ctime>
#include <iostream>
#include <mutex>
#include <sstream>
#include <sys/timeb.h>
#include "LockFreeQueue.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	TaskManager
///
/// @brief	작업 관리를 위한 클래스
////////////////////////////////////////////////////////////////////////////////////////////////////
class TaskManager
	:
	public boost::noncopyable // 복사를 막기 위해 씀!
{
	// boost::noncopyable 를 상속받으면 아래 두 줄을 private로 선언 안해도 복사 막을 수 있음!
	//TaskManager( const TaskManager& );
	//const TaskManager& operator=( const TaskManager& ){};


public:
	/// 작업 타입 정의
	typedef std::function< void() > Task;

	///< 작업 타입 정의
	typedef Task TaskExt;

public:
	/// 고정 작업 큐 타입 정의
	typedef LockFreeQueue< TaskExt*, 256 > FixedTaskQueue;

	/// 가변 작업 큐 타입 정의
	typedef LockFreeQueue< TaskExt* > VariantTaskQueue;

	/// 작업 처리 함수 타입 정의
	typedef std::function< void( TaskExt* ) > TaskConsumer;

private:
	std::string             m_name;                 ///< 이름
	int                     m_threadId;             ///< 쓰레드 식별자
	std::mutex              m_mutex;            ///< 대기 뮤텍스
	std::condition_variable m_conditionVariable;    ///< 조건 변수
	FixedTaskQueue          m_taskPool;             ///< 작업 객체 풀
	FixedTaskQueue          m_taskQueue;            ///< 작업 큐
	VariantTaskQueue        m_subTaskQueue;         ///< 보조 작업 큐
	TaskConsumer            m_taskConsumer;         ///< 작업 처리자
	bool                    m_hasTask;              ///< 태스크 소유 여부
	std::atomic_int         m_taskCount;            ///< 태스크 개수
	bool                    m_logEnabled;           ///< 로그 활성화 여부
	__int64                 m_nextLogTime;          ///< 다음 로그 시각
	bool                    m_useConditionVariable; ///< 조건 변수 사용 여부

public:
	/// 생성자
	TaskManager();

	/// 소멸자
	~TaskManager();

	// 초기화한다.
	void Initialize( const std::string& name, int threadId, bool useConditionVariable = true );

	/// 이 객체를 처음 상태로 되돌린다.
	void Reset( bool logEnabled = false, void* ownerPtr = nullptr, const std::string& ownerInfo = "" );

	/// 쓰레드를 설정한다.
	void SetThreadId( int threadId ) {
		m_threadId = threadId;
	}

	/// 작업 처리자를 설정한다.
	void SetTaskConsumer( const TaskConsumer& taskConsumer ) {
		m_taskConsumer = taskConsumer;
	}

	/// 로그 활성화 여부를 설정한다.
	void SetLogEnabled( bool logEnabled ) {
		m_logEnabled = logEnabled;
	}

	/// 이름을 반환한다.
	const std::string& GetName() const {
		return m_name;
	}

	/// 쓰레드를 반환한다.
	int GetThreadId() const {
		return m_threadId;
	}

	/// 작업 처리자를 반환한다.
	const TaskConsumer& GetTaskConsumer() const {
		return m_taskConsumer;
	}

	/// 태스크 개수를 반환한다.
	int GetTaskCount() const;

	/// 대상 쓰레드에서 작업을 실행한다.
	void RunTask( const Task& task, bool blocking = false );

	/// 갱신한다.
	int Update( const __int64& curTime );

	/// 작업이 생길 때까지 대기 후 갱신한다.
	void WaitAndUpdate();

private:
	/// 작업을 넣는다.
	void _PushTask( TaskExt* taskExt );

	int _GetCurrentThreadId();

private:
	static bool ms_useConditionVariable; ///< 조건 변수 사용 여부

public:
	/// 조건 변수 사용 여부를 설정한다.
	static void SetUseConditionVariable( bool useConditionVariable ) {
		ms_useConditionVariable = useConditionVariable;
	}
};
