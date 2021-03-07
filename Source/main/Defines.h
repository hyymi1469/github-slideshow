#pragma once


#define GAME_SERVER_INI_PATH    "./GameServer.ini"  // GameServer.ini를 읽어 올 경로를 설정한다.
#define MAX_LISTEN_COUNT	    1                   // 연결할 listen 갯수를 설정한다.( WSA_MAXIMUM_WAIT_EVENTS 갯수 제한으로 인해 64개 이하로 설정할 것)
#define FULL_USER_OBJ_POOL      60000               // 최대 유저 오브젝트 풀을 설정한다.
#define USER_OBJ_POOL           2000               // 유저 오브젝트 풀을 설정한다.
#define EXTRA_USER_OBJ_POOL     200                 // 유저 오브젝트 풀을 추가로 설정한다.
#define BUFFER_SIZE             9600                // 송수신 버퍼 사이즈(컴퓨터가 읽기 쉽도록 16진수로 한다.)
#define	PACKET_HEADERSIZE       6                   // 패킷 해더 사이즈
#define	PACKET_START_POS        0                   // 패킷 읽기 시작 지점
#define	PACKET_HEADER_READ      2                   // 패킷 헤더 읽기 지점
#define	DISCONNECT_SESSION_SEC  180                 // 세션을 끊을 최소 시간( 초 단위 )