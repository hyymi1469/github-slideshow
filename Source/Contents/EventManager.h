#pragma once
#include <map>
#include <time.h> // tmep ymi
#include "../main/Singleton.h"

class EventManager
	:
	public Singleton< EventManager >
{
	// 이벤트 테스트 맵<sessionId, eventCount >
	typedef long long Int64;
	typedef std::map< int, Int64 > TestEventMap;

private:
	TestEventMap m_testMap; // 락프리 테스트를 위한 이벤트 맵
	clock_t      m_start;   // temp ymi
	clock_t      m_end;     // temp ymi

public:
	// 생성자
	EventManager();

	// 소멸자
	virtual ~EventManager();

	// 테스트용 맵의 사이즈를 반환한다.
	int GetTestMapSize();

	// 이벤트 맵에 해당 키와 값을 추가한다.
	void EmplaceTestMap( const int key, const int value );

	// 시간 기록을 반환한다.
	double GetTestTime();

	TestEventMap& GetTestMap()
	{
		return m_testMap;
	};
};

