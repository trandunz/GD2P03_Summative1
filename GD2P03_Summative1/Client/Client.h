#pragma once
#include "Global.h"
class Client
{
public:
	Client();
	~Client();

private:
	void RecieveFromServer();
	void SendToServer();

	void ConnectToServer(std::string _ip);

	int m_ClientSocket;
	int m_Status;
	char m_InBuffer[BUFFER_SIZE]{};
	char m_OutBuffer[BUFFER_SIZE]{};

	std::mutex g_mutex;
	std::condition_variable g_cv;
	bool m_IsConnected{ false };

	std::vector<std::thread> m_ThreadPool;
	bool m_ServerWaitingForCommand = true;
};

