#pragma once
#include <memory>
#include "Singleton.h"
#include "TaskManager.h"


class Logic
	:
	public Singleton< Logic >
{
private:
	//TaskManager m_taskManager; // 작업 관리자

public:
	// 생성자
	Logic();

	// 소멸자
	virtual ~Logic();

};


typedef std::shared_ptr< Logic > LogicPtr;
