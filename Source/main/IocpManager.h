#pragma once
#include <winsock2.h>
#include "Singleton.h"


class IocpManager
	:
	public Singleton< IocpManager >
{
private:
	HANDLE m_iocpHandle;  // IOCP 핸들

public:
	// 생성자
	IocpManager();

	// 소멸자
	virtual ~IocpManager();

	// IOCP 핸들, Worker Thread Pool 생성하는 메서드
	bool Initialize();

	// 핸들과 소켓 핸들을 연결한다.
	bool Associate( HANDLE hDevice, ULONG_PTR pCompletionKey );

	// m_iocpHandle를 반환한다.
	HANDLE GetWorkerIOCPHandle();
};
