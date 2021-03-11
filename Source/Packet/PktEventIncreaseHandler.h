#pragma once
#include "Packet.h"
#include <mutex> // test ymi


class User;
class PktEventIncreaseHandler
{
private:
	std::mutex m_mutex;
public:
	void OnHandler( Packet& packet, User* user );
};

