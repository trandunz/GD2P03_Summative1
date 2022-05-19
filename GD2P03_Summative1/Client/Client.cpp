#include "Client.h"

Client::Client()
{
	InitWSA();
	Init(5000);
}

Client::~Client()
{
	// Join all the sending and recieving threads
	for (auto& thread : m_ThreadPool)
	{
		if (thread.joinable())
			thread.join();
	}
	m_ThreadPool.clear();
	// Close the connection if its not already
	closesocket(m_ClientSocket);
	// Cleanup WSA
	WSACleanup();
}

void Client::Init(u_short _port)
{
	// Create a new socket for sending and recieving data.
	m_ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	// Check if it failed
	if (m_ClientSocket == -1)
	{
		printf("Error: Socket() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}

	// Prompts the user to enter the server ip adress
	printf("Enter Server IP: ");
	std::cin >> m_ServerIP;

	ConnectToServer(m_ServerIP, _port);
	// If connection worked, Start two threads that recieve and send to the server.
	if (m_IsConnected)
	{
		m_ThreadPool.emplace_back(std::thread(&Client::RecieveFromServer, this));
		m_ThreadPool.emplace_back(std::thread(&Client::SendToServer, this));
	}
	// While the connection fails, retry or quit program
	while (!m_IsConnected)
	{
		printf("Would You Like To Retry Connection? (1/0)\n");
		ReconnectionCheck(_getch());
		return;
	}
}

void Client::RecieveFromServer()
{
	auto status{0};
	do 
	{
		// Reset the inBuffer 
		memset(m_InBuffer, 0, sizeof(m_InBuffer));

		// Recieve the data from the server into the inBuffer
		status = recv(m_ClientSocket, m_InBuffer, BUFFER_SIZE, 0);

		// Check for server disconnection or errors.
		if (WSAGetLastError() == WSAECONNRESET)
		{
			printf("Client: Disconnection From Server %s\n", GetIpFromSocket(std::move(m_ClientSocket)));
			m_IsConnected = false;
			printf("Would You Like To Retry Connection? (1/0)\n");
			return;
		}
		else if (status == -1)
		{
			printf("recv Error!\n");
			return;
		}
		else
		{
			// Apply null termination character at the end of the message
			if (sizeof(m_InBuffer) / sizeof(char) > status)
				m_InBuffer[status] = '\0';

			// Print the recieved message
			printf("%s\n", m_InBuffer);
		}
	} while (status > 0);
}

void Client::SendToServer()
{
	auto status{0};
	std::string message{};
	do
	{
		if (m_IsConnected)
		{
			// Reset the outBuffer
			memset(m_OutBuffer, 0, sizeof(m_OutBuffer));

			// Get a message from the user
			std::getline(std::cin, message);
			// if its too large shrink it
			if (message.length() >= BUFFER_SIZE - 1)
			{
				message.erase(BUFFER_SIZE - 1);
			}
			message.shrink_to_fit();
			// Copy it to the outBuffer
			strcpy_s(m_OutBuffer, message.c_str());

			// Check if reconnection needs to occur.
			if (ReconnectionCheck(std::move(m_OutBuffer[0])))
				return;

			// Send the data to the server
			status = send(m_ClientSocket, m_OutBuffer, strlen(m_OutBuffer), 0);
			// Check for errors.
			if (status == -1)
			{
				printf("Send Error!\n");
				return;
			}
			// If the client sends quit, exit program
			if ((std::string)m_OutBuffer == "QUIT")
			{
				Sleep(1000);
				exit(0);
			}
		}
	} while (true);
}

void Client::ConnectToServer(std::string _ip, u_short _port)
{
	// create a container for ip and port of the server
	sockaddr_in receiverAddr{0};
	receiverAddr.sin_family = AF_INET;
	receiverAddr.sin_port = htons(_port);
	receiverAddr.sin_addr.s_addr = inet_addr(_ip.data());
	
	// Attempts a connection to the server
	printf("Attempting Connection To %s\n", _ip.c_str());
	auto status = connect(m_ClientSocket, (sockaddr*)&receiverAddr, sizeof(sockaddr));
	// If an error occured, keep isconnected false else isConnected becomes true
	if (status == -1)
	{
		printf("Connection To %s Failed!\n", _ip.data());
		m_IsConnected = false;
	}
	else
	{
		m_IsConnected = true;
	}
}

char* Client::GetIpFromSocket(int&& _socket)
{
	// Create a container for the socketInfo
	SOCKADDR_IN client_info = { 0 };
	int addrsize = sizeof(client_info);
	// Get the information from the socket and put it into client_info
	getpeername(_socket, (sockaddr*)&client_info, &addrsize);
	// Returns the ip adress as a string
	return inet_ntoa(client_info.sin_addr);
}

bool Client::ReconnectionCheck(char&& _input)
{
	if (!m_IsConnected)
	{
		// If the user wishes to reconnect then close the socket and restart the whole connection process.
		if (_input == '1')
		{
			closesocket(m_ClientSocket);
			Init(5000);
			return true;
		}
		// If the user does not want to reconnect, close the program
		else
		{
			printf("Closing Program...");
			Sleep(1000);
			exit(0);
		}
	}
	return false;
}
