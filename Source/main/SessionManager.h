#pragma once
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <WinSock2.h>
#include "Defines.h"
#include "LockFreeQueue.h"
#include "Session.h"
#include "Singleton.h"


class User;
class SessionManager
	:
	public Singleton< SessionManager >
{
private:

	// 유저 동적할당 풀
	typedef LockFreeQueue< Session*, FULL_USER_OBJ_POOL > UserObjectPoolQue;

	// 활동 중인 맵
	typedef std::map< INT64, Session* >                   ActiveObjectPoolMap;

private:
	UserObjectPoolQue    m_userObjectPoolQue;   // 유저 오프젝트 풀을 미리 생성하고 담아 놓는 큐
	ActiveObjectPoolMap  m_activeObjectPoolMap; // 활동 중인 유저 오브젝트를 담아 놓는 맵
	std::mutex           m_mutex;               // 뮤텍스
	std::atomic< INT64 > m_curMaxSessionNum;    // 현재 세션 최고 수

public:
	// 생성자
	SessionManager();

	// 소멸자
	virtual ~SessionManager();

	// 유저풀을 생성한다.
	void Initialize();

	// 새로운 Session 객체를 얻는다.
	void CreateNewSession( SOCKET socket );

	// 오브젝트를 반환한다.
	void PushObjPool( User* user );

	// 마지막 패킷 수신 시간을 확인 후 세션을 자른다.
	void CheckAndDisconnect();

	// 활동 중인 오브젝트를 뺀다.
	void PopActiveObject( const INT64 activeSessionNum );

private:
	// 오브젝트 풀을 넣는다.
	void _PushObjPool( const int objCount );

	// 유저풀에서 꺼낼 떄 풀이 비어있으면 추가로 200개 풀을 넣는다.
	User* _PopObjPool();
};

