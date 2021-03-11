#include <iostream>
#include <cstdio>
#include <unordered_map>
#include "Environment.h"
#include "SessionManager.h"
#include "Singleton.h"
#include "IocpManager.h"
#include "SocketManager.h"
#include "Enum.h"
#include "Defines.h"

#pragma warning(disable : 4996)

typedef std::unordered_map< std::string, std::string > IniStrMap;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	환경변수를 설정한다.
///
/// @param	iniStrMap  적용한 환경변수 맵
/// 
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
bool _SetEnvironment( const IniStrMap& iniStrMap )
{
	auto iter = iniStrMap.find( "ServerIP" );
	if ( iter == iniStrMap.end() )
		return false;
	Environment::GetSingleton()->SetGameServerIp( iter->second );

	iter = iniStrMap.find( "Port" );
	if ( iter == iniStrMap.end() )
		return false;
	Environment::GetSingleton()->SetGameServerPort( atoi( iter->second.c_str() ) );

	iter = iniStrMap.find( "LogicThreadCount" );
	if ( iter == iniStrMap.end() )
		return false;
	Environment::GetSingleton()->SetLogicThreadCount( atoi( iter->second.c_str() ) );

	iter = iniStrMap.find( "IoThreadCount" );
	if ( iter == iniStrMap.end() )
		return false;
	Environment::GetSingleton()->SetIoThreadCount( atoi( iter->second.c_str() ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	시작
///
/// @return	없음
////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
	// 1. GameServer를 구성할 ini파일을 불러오고 셋 해준다.
	// ini불러오기 함수는 windows함수기 때문에 리눅스에서 안돌아갈 수 있으므로 fopen사용
	FILE* file = fopen( GAME_SERVER_INI_PATH, "r" );
	if ( !file )
		return printf("GameServer.ini Read Error!!!\n");

	IniStrMap iniStrMap;
	while ( feof( file ) == 0 )
	{
		char str[ 200 ] = "";
		std::fgets( str, 200, file );

		std::string tempStr = str;
		int index = tempStr.find( "=" );
		if ( index < 0 )
			continue;

		std::string tempStrPair1 = tempStr.substr( 0, index );
		std::string tempStrPair2 = tempStr.substr( index + 1, tempStr.size() );

		iniStrMap.emplace( tempStrPair1, tempStrPair2 );
	}

	if ( !_SetEnvironment( iniStrMap ) )
		return false;

	// 2. IOCP를 위한 사전 준비를 한다.(IO포트 생성, 워커 스레드풀 생성)
	if ( !IocpManager::GetSingleton()->Initialize() )
		return printf("IocpManager Initialize Failed!");

	// 4. 동적할당 유저풀 생성
	SessionManager::GetSingleton()->Initialize();

	// 5. Accept스레드 만들고 Accept시작
	if ( !SocketManager::GetSingleton()->Initialize( ESocketType::Tcp ) )
		return printf( "SocketManager Initialize Failed!" );


	// 메인 스레드가 안끝나도록 대기
	HANDLE writeEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	WaitForSingleObject( writeEvent, INFINITE );     // 쓰기 완료를 기다림(Write이벤트 끝날때까지 대기)


	Environment::Release();
	return 0;
} 
