#pragma once
#include "Packet.h"


class User;
class PktTestHandler
{
public:
	static void OnHandler( Packet& packet, User* user );
};

