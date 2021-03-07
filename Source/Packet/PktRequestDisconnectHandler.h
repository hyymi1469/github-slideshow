#pragma once
#include "Packet.h"


class User;
class PktRequestDisconnectHandler
{
public:
	static void OnHandler( Packet& packet, User* user );
};

