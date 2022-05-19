#pragma once
#include "Global.h"
class Server
{
public:
	/// <summary>
	/// Server Constructor.
	/// Starts the server.
	/// </summary>
	Server();

	// Server Destructor
	~Server();

private:
	// Begins the connection process on specified port.
	void Init(u_short _port);

	/// <summary>
	/// Listens out for connections from clients.
	/// </summary>
	void Listen();

	/// <summary>
	/// Accepts connections from clients and
	/// Assigns them a new thread.
	/// </summary>
	void Accept();

	/// <summary>
	/// Handles sending and receving on data to and from the specified client socket.
	/// </summary>
	/// <param name="_client"></param>
	void SendAndRecieve(int _client);

	/// <summary>
	/// Sends a message to the specified client.
	/// </summary>
	/// <param name="_client"></param>
	/// <param name="_message"></param>
	void SendMessageToClient(int _client, std::string _message);
	/// <summary>
	/// Prints the ip of the specified client and the command sent.
	/// </summary>
	/// <param name="_client"></param>
	/// <param name="_incomingCommand"></param>
	void PrintIPSentCommand(int _client, std::string _incomingCommand);

	/// <summary>
	/// Checks if the given command is valid and returns its command index inside the command list.
	/// </summary>
	/// <param name="_client"></param>
	/// <param name="_buffer"></param>
	/// <param name="_executingCommand"></param>
	/// <param name="_incomingCommand"></param>
	/// <returns></returns>
	int CheckValidCommand(int _client, char* _buffer, bool& _executingCommand,std::string _incomingCommand);

	/// <summary>
	/// Returns the ip from the specified socket.
	/// </summary>
	/// <param name="_socket"></param>
	/// <returns></returns>
	char* GetIpFromSocket(int _socket);

	int m_ServerSocket;
	int m_ActiveCommandIndex{};
	char m_Buffer[BUFFER_SIZE]{};

	std::map<int, std::string> m_SavedMesssages{};

	std::vector<std::thread> m_ThreadPool;

	std::vector <Command> m_Commands{};
};
