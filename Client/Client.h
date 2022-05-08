#pragma once
#include "Global.h"
class Client
{
public:
	Client();
	~Client();

	void Update();

private:
	int m_ClientSocket;
	int m_Status;
	char m_Buffer[BUFFER_SIZE]{};
	bool m_ServerWaitingForCommand = true;
};

