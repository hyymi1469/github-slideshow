#pragma once


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷 프로토콜을 선언한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class Protocol : int
{
	None = 0,

	// 테스트용 패킷
	Test       = 1,
	TestResult = 2,

	// 이벤트 증가
	EventIncrease       = 3,
	EventIncreaseResult = 4,

	// 이벤트 감소
	EventDecrease       = 5,
	EventDecreaseResult = 6,

	// 연결 해제 요청
	RequestDisconnect       = 7,
	RequestDisconnectResult = 8,
};