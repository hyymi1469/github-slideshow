#pragma once
#include "Thread.h"


class Accepter
	:
	public Thread
{
public:
	// 생성자
	Accepter();

	// 소멸자
	virtual ~Accepter();

	// 스레드 생성을 시작한다.
	void Begin() override;

	// 스레드를 작동시킨다.
	void Run() override;
};


typedef std::shared_ptr< Accepter > AccepterPtr;
