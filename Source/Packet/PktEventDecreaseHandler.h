#pragma once
#include "Packet.h"


class User;
class PktEventDecreaseHandler
{
public:
	static void OnHandler( Packet& packet, User* user );
};

