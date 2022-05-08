#pragma once
#include "Global.h"
class Server
{
public:
	Server();
	~Server();

	void Init(int _port);
	void Update();

private:
	void Listen();
	void Accept();

	void SendMessageToClient(std::string _message);
	void PrintIPSentCommand(std::string _incomingCommand);

	int CheckValidCommand(std::string _incomingCommand);

	int m_ServerSocket;
	int m_ListenerSocket;
	int m_Status;
	int m_ActiveCommandIndex{};
	char m_Buffer[BUFFER_SIZE]{};

	bool m_IsConnected = false;
	bool m_IsExecutingCommand = false;
	std::string ActiveCommand{};

	std::string savedMessage{};

	std::vector <std::pair<std::string, std::function<void(bool _finished)>>> m_Commands{};
};

