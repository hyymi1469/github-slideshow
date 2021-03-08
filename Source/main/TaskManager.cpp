#include "TaskManager.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @param	name					이름
/// @param	thread					쓰레드
/// @param	useConditionVariable	조건 변수 사용 여부
////////////////////////////////////////////////////////////////////////////////////////////////////
TaskManager::TaskManager()
	:
	m_hasTask( false ),
	m_taskCount( 0 ),
	m_logEnabled( false ),
	m_nextLogTime( 0 )
{
	m_taskConsumer = [ this ] ( TaskExt* taskExt )
	{
		( *taskExt )( );

		*taskExt = nullptr;

		if ( !m_taskPool.push( taskExt ) )
		{
			delete taskExt;
			taskExt = nullptr;
		}

		m_taskCount.fetch_sub( 1 );

	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
/// 
/// @param	name					이름
/// @param	thread					쓰레드
/// @param	useConditionVariable	조건 변수 사용 여부
/// 
/// @return	None
////////////////////////////////////////////////////////////////////////////////////////////////////
void TaskManager::Initialize( const std::string& name, int threadId, bool useConditionVariable/* = true*/ )
{
	m_name = name;
	m_threadId = threadId;
	m_useConditionVariable = useConditionVariable;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @breif	소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
TaskManager::~TaskManager()
{
	Reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	이 객체를 처음 상태로 되돌린다.
///
/// @param	logEnabled	로그 활성화 여부
/// @param	ownerPtr	소유자 포인터
/// @param	ownerInfo	소유자 정보
///
/// @return	반환 값 없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void TaskManager::Reset( bool logEnabled /*= false*/, void* ownerPtr /*= nullptr*/, const std::string& ownerInfo /*= ""*/ )
{
	if ( !logEnabled )
	{
		m_taskQueue.consume_all( std::default_delete< TaskExt >() );
	}
	else
	{
		m_taskQueue.consume_all( [ownerPtr, ownerInfo] ( TaskExt* task )
								 {
									 delete task;
									 task = nullptr;
								 } );
	}

	m_taskPool.consume_all( std::default_delete< TaskExt >() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	태스크 개수를 반환한다.
///
/// @return	태스크 개수
////////////////////////////////////////////////////////////////////////////////////////////////////
int TaskManager::GetTaskCount() const
{
	return m_taskCount.load();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	대상 쓰레드에서 작업을 실행한다.
///
/// @param	task		작업
/// @param	callerInfo	호출자 정보
/// @param	blocking	블러킹 여부
///
/// @return	반환 값 없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void TaskManager::RunTask( const Task& task, bool blocking/* = false*/ )
{
	if ( blocking && m_threadId == _GetCurrentThreadId() )
	{
		task();
		return;
	}

	TaskExt* taskPtr = nullptr;
	if ( !m_taskPool.pop( taskPtr ) )
		taskPtr = new TaskExt();

	if ( !blocking )
	{
		*taskPtr = task;

		_PushTask( taskPtr );
		return;
	}

	std::atomic< bool > completion( false );
	*taskPtr = [&task, &completion] ()
	{
		task();
		completion.store( true );
	};

	_PushTask( taskPtr );

	while ( !completion.load() )
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	갱신한다.
///
/// @param	curTime	현재 시각
/// 
/// @return	처리한 작업 수
////////////////////////////////////////////////////////////////////////////////////////////////////
int TaskManager::Update( const __int64& curTime )
{
	if ( m_logEnabled && m_nextLogTime <= curTime )
	{
		int taskCount = GetTaskCount();
		if ( taskCount )
		{
			printf( "[TaskManager::Update] [name: %s, taskCount: %d]", m_name.c_str(), GetTaskCount() );
		}
		m_nextLogTime = curTime + 1000 * 10;
	}

	int taskCount = 0;

	taskCount += m_taskQueue.consume_all( m_taskConsumer );
	taskCount += m_subTaskQueue.consume_all( m_taskConsumer );

	if ( m_useConditionVariable )
		m_hasTask = ( m_taskCount > 0 );

	return taskCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	작업이 생길 때까지 대기 후 갱신한다.
///
/// @return	반환 값 없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void TaskManager::WaitAndUpdate()
{
	if ( m_useConditionVariable )
	{
		if ( m_taskQueue.empty() && m_subTaskQueue.empty() )
		{
			std::unique_lock< std::mutex > lock( m_mutex );

			m_conditionVariable.wait(
				lock,
				[this] ()
				{
					return m_hasTask;
				} );
		}
	}

	struct timeb tb;
	ftime( &tb );
	const __int64 curTime = ( std::time( nullptr ) * 1000 ) + tb.millitm;
	Update( curTime );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	작업을 넣는다.
///
/// @param	taskExt	확장 작업
///
/// @return	반환 값 없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void TaskManager::_PushTask( TaskExt* taskExt )
{
	bool result = false;
	if ( m_subTaskQueue.empty() )
		result = m_taskQueue.push( taskExt );

	if ( !result )
		while ( !m_subTaskQueue.push( taskExt ) )
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

	m_taskCount.fetch_add( 1 );
	if ( m_useConditionVariable )
	{
		{
			std::unique_lock< std::mutex > lock( m_mutex );
			m_hasTask = true;
		}

		m_conditionVariable.notify_one();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	현재 스레드의 id값을 받아 온
///
/// @param	taskExt	확장 작업
///
/// @return	반환 값 없음
////////////////////////////////////////////////////////////////////////////////////////////////////
int TaskManager::_GetCurrentThreadId()
{
	std::stringstream ss;
	ss << std::this_thread::get_id();
	return atoi( ss.str().c_str() );
}
