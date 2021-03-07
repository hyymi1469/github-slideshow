#pragma once
#include <map>
#include "../main/Singleton.h"

class EventManager
	:
	public Singleton< EventManager >
{
	// 이벤트 테스트 맵<sessionId, eventCount >
	typedef long long Int64;
	typedef std::map< int, Int64 > TestEventMap;

private:
	TestEventMap m_testEventMap; // 락프리 테스트를 위한 이벤트 맵

public:
	// 생성자
	EventManager();

	// 소멸자
	virtual ~EventManager();

	// 이벤트 맵에 해당 키와 값을 추가한다.
	void SetEventMapCount( const int key, const int value );

};

