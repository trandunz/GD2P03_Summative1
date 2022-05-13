#pragma once
#include "Global.h"
class Server
{
public:
	Server();
	~Server();

	void Init(int _port);

private:

	void Listen();
	void Accept();

	void SendAndRecieve(int _client);

	void SendMessageToClient(int _client, std::string _message);
	void PrintIPSentCommand(int _client, std::string _incomingCommand);

	int CheckValidCommand(int _client, std::string _incomingCommand);

	char* GetIpFromSocket(int _socket);

	int m_ServerSocket;
	int m_Status;
	int m_ActiveCommandIndex{};
	char m_Buffer[BUFFER_SIZE]{};

	bool m_IsConnected = false;
	bool m_IsExecutingCommand = false;
	std::string ActiveCommand{};

	std::string savedMessage{};

	std::vector<int> m_ConnectedSockets;
	std::vector<std::thread> m_ThreadPool;

	std::vector <Command> m_Commands{};
};

