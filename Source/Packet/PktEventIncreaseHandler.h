#pragma once
#include "Packet.h"


class User;
class PktEventIncreaseHandler
{
public:
	static void OnHandler( Packet& packet, User* user );
};

