#pragma once
#include "Global.h"
class Client
{
public:
	/// <summary>
	/// Client Constructor. Starts the client.
	/// </summary>
	Client();

	// Client destructor.
	~Client();

private:
	/// <summary>
	/// Begins the connection process on specified port.
	/// </summary>
	/// <param name="_port"></param>
	void Init(u_short _port);

	/// <summary>
	/// Recieves data from the server into the m_InBuffer.
	/// </summary>
	void RecieveFromServer();

	/// <summary>
	/// Sends data to the server, storing sent data in the m_OutBuffer.
	/// </summary>
	void SendToServer();

	/// <summary>
	/// Attempts a connection to the specified ip and port.
	/// </summary>
	/// <param name="_ip"></param>
	/// <param name="_port"></param>
	void ConnectToServer(std::string _ip, u_short _port);
	
	/// <summary>
	/// Returns the ip of the given socket.
	/// </summary>
	/// <param name="_socket"></param>
	/// <returns></returns>
	char* GetIpFromSocket(int&& _socket);

	/// <summary>
	/// Handles reconnecting to the server or closing the program if not.
	/// </summary>
	/// <param name="_input"></param>
	/// <returns></returns>
	bool ReconnectionCheck(char&& _input);

	int m_ClientSocket{};
	std::string m_ServerIP{};
	char m_InBuffer[BUFFER_SIZE]{};
	char m_OutBuffer[BUFFER_SIZE]{};

	bool m_IsConnected{ false };

	std::vector<std::thread> m_ThreadPool;
};

