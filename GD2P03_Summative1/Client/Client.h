#pragma once
#include "Global.h"
class Client
{
public:
	Client();
	~Client();

private:
	void Init();

	void RecieveFromServer();
	void SendToServer();

	void ConnectToServer(std::string _ip, u_short&& _port);
	char* GetIpFromSocket(int&& _socket);

	bool ReconnectionCheck(char&& _input);

	int m_ClientSocket{};
	int m_ServerPort{};
	std::string m_ServerIP{};
	char m_InBuffer[BUFFER_SIZE]{};
	char m_OutBuffer[BUFFER_SIZE]{};

	std::mutex g_mutex;
	std::condition_variable g_cv;
	bool m_IsConnected{ false };

	std::vector<std::thread> m_ThreadPool;
};

