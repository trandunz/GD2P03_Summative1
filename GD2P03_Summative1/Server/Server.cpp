#include "Server.h"

Server::Server()
{
	InitWSA();

	// Add CAPITALIZE to command list and specify functionality
	m_Commands.emplace_back(Command{ "CAPITALIZE", [this](int _client,char* _buffer,bool _finished)
		{
			// If user sent "."
			if (_finished)
			{
				// Concatinate the string to the buffer size
				if (m_SavedMesssages[_client].length() >= BUFFER_SIZE)
				{
					m_SavedMesssages[_client].erase(BUFFER_SIZE - 1);
				}
				// Remove '.'
				m_SavedMesssages[_client].pop_back();

				// Convert the  message to uppercase
				std::string upperString{};
				for (auto& character : m_SavedMesssages[_client])
				{
					upperString += toupper(character);
				}
				// Send the uppercase message to the client
				SendMessageToClient(_client,upperString);
			}
			else if (m_SavedMesssages[_client].length() < BUFFER_SIZE)
			{
				// If the user is still entering the message, add it to there saved message.
				m_SavedMesssages[_client] += _buffer;
				m_SavedMesssages[_client] += '\n';
			}

		} });

	// Add PUT to command list and specify functionality
	m_Commands.emplace_back(Command{ "PUT", [this](int _client,char* _buffer,bool _finished)
		{
			// If user sent "."
			if (_finished)
			{
				// Concatinate the string to the buffer size
				if (m_SavedMesssages[_client].length() >= BUFFER_SIZE)
				{
					m_SavedMesssages[_client].erase(BUFFER_SIZE - 1);
				}
				// Remove '.'
				m_SavedMesssages[_client].pop_back();
				
				// Send 200 Ok to confirm message has been saved
				SendMessageToClient(_client, "200 OK");
			}
			else if (m_SavedMesssages[_client].length() < BUFFER_SIZE)
			{
				// If the user is still entering the message, add it to there saved message.
				m_SavedMesssages[_client] += _buffer;
				m_SavedMesssages[_client] += '\n';
			}
		} });

	// Add GET to command list and specify functionality
	m_Commands.emplace_back(Command{ "GET", [this](int _client,char* _buffer,bool _finished)
		{
			// Returns the clients saved message
			SendMessageToClient(_client,m_SavedMesssages[_client]);
		}, true });

	// Add QUIT to command list and specify functionality
	m_Commands.emplace_back(Command{ "QUIT", [this](int _client,char* _buffer,bool _finished)
		{
			// Has no functionality but is added so server can confirm its a valid command.
			// And send 200 OK
		}, true });

	// Add TIME to command list and specify functionality
	m_Commands.emplace_back(Command{ "TIME", [this](int _client,char* _buffer,bool _finished)
		{
			time_t t = time(NULL);
			struct tm buf;
			char str[30];
			localtime_s(&buf, &t);
			asctime_s(str,sizeof(str), &buf);
			std::string removedNewLineStr = str;
			removedNewLineStr.pop_back();
			SendMessageToClient(_client, removedNewLineStr);
		}, true });

	Init(5000);
}

Server::~Server()
{
	// Join all the client threads
	for (auto& thread : m_ThreadPool)
	{
		if (thread.joinable())
			thread.join();
	}
	m_ThreadPool.clear();

	// Close all the client sockets
	for (auto& socket : m_SavedMesssages)
	{
		closesocket(socket.first);
	}
	// Close the server socket
	closesocket(m_ServerSocket);
	// Cleanup WSA
	WSACleanup();
}

void Server::Init(u_short _port)
{
	// Creatte the server socket
	m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	// Check for errors
	if (m_ServerSocket == -1)
	{
		printf("Error: Socket() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}

	// Create the serverAddr container
	struct sockaddr_in serverAddr { AF_INET , htons(_port), INADDR_ANY };

	auto status{0};
	// Bind the serverInformation to the server socket (e.g port, protocol type e.t.c)
	status = bind(m_ServerSocket, (sockaddr*)&serverAddr, sizeof(sockaddr));
	// Check for errors
	if (status == 1)
	{
		printf("Error: Bind() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}
	
	// Listen out for connections
	Listen();
	// Accept connections and open a new thread for them
	Accept();
}

void Server::Listen()
{
	// Listen out with 5 as max queue of 5 to be accepted
	printf("Listening for connections...\n");
	auto status{0};
	status = listen(m_ServerSocket, SOMAXCONN);
	if (status == -1)
	{
		printf("Error: Listen() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}
}

void Server::Accept()
{
	// Create a container for client info
	struct sockaddr_in client_info {0};
	int length = sizeof(client_info);
	auto client{ 0 };
	while (client = accept(m_ServerSocket, (sockaddr*)&client_info, &length))
	{
		// Check for errors
		if (client == INVALID_SOCKET)
		{
			printf("Invalid Client Socket\n");
			continue;
		}

		// Make a new client specific savedmessage
		m_SavedMesssages.insert(std::make_pair(client, ""));

		// Open a new thread for the client
		m_ThreadPool.emplace_back(std::thread(&Server::SendAndRecieve, this, client));
	}
}

void Server::SendAndRecieve(int _client)
{
	printf("Server: Connection From Client %s\n", GetIpFromSocket(_client));

	// Upon Connection, Send Server is ready
	SendMessageToClient(_client, "Server is ready...");

	// Create unique buffer and other variables needed for individual command usage
	char buffer[BUFFER_SIZE];
	auto status{0};
	bool executingCommand{ false };
	int activeCommandIndex{ -1 };
	// Recieve from the server 
	while (status = recv(_client, buffer, BUFFER_SIZE, 0))
	{
		// Set charactter after message to null termination
		if (sizeof(buffer) / sizeof(char) > status)
			buffer[status] = '\0';

		// Check for disconnection or error
		if (status == -1)
		{
			int error = WSAGetLastError();
			if (error == WSAECONNRESET)
			{
				printf("Server: Disconnection From Client %s\n", GetIpFromSocket(_client));
			}
			else
			{
				printf("Error: recv() failed. Code: %d\n", error);
			}
			return;
		}
		// If no error,
		else
		{
			// convert the buffer into a string
			std::string incomingMessage{ buffer };

			// If no command is being executed, check if its a valid command.
			// If its set to immediate, execute it.
			// If its not immediate, extract index in command list
			if (!executingCommand)
			{
				activeCommandIndex = CheckValidCommand(_client, buffer, executingCommand, incomingMessage);
			}
			// If running command is executing and activeCommandIndex is valid
			else if (executingCommand && activeCommandIndex >= -1)
			{
				// If message finished, run functionality with _finished set to true
				if (incomingMessage == ".")
				{
					m_Commands[activeCommandIndex].function(_client, buffer, true);
					// Stop executing the command
					executingCommand = false;
					// Reset command index
					activeCommandIndex = -1;
				}
				// Else if message isent finsihed, run functionality with _finished set to false
				else
				{
					m_Commands[activeCommandIndex].function(_client, buffer, false);
				}
			}
		}

		// reset the buffer
		memset(m_Buffer, 0, sizeof(m_Buffer));
	}
	
}

void Server::SendMessageToClient(int _client, std::string _message)
{
	send(_client, _message.data(), _message.length(), 0);
}

void Server::PrintIPSentCommand(int _client, std::string _incomingCommand)
{
	// create client info container
	SOCKADDR_IN client_info{ 0 };
	int size = sizeof(client_info);
	// populate client info container with ip, e.t.c
	getpeername(_client, (sockaddr*)&client_info, &size);
	// Print ip sends command
	std::cout << inet_ntoa(client_info.sin_addr) << " sends " << _incomingCommand << std::endl;
}

int Server::CheckValidCommand(int _client, char* _buffer,bool& _executingCommand, std::string _incomingCommand)
{
	auto i{ 0 };
	// Loop through all commands
	for (auto& command : m_Commands)
	{
		// if name matches
		if (_incomingCommand == command.name)
		{
			// Send 200 OK
			SendMessageToClient(_client,"200 OK");
			// Print on server side whatt command from what ip
			PrintIPSentCommand(_client,_incomingCommand);

			// If the command is immediate, run it and set executing command false
			if (command.immediate)
			{
				command.function(_client, _buffer,true);
				_executingCommand = false;
			}
			// Else, clear the clients saved message and set executing command true
			else
			{
				m_SavedMesssages[_client].clear();
				_executingCommand = true;
			}
			return i;
		}
		i++;
	}
	
	// if the command was not recognised, send 400 Command not valid
	SendMessageToClient(_client,"400 Command not valid.");
	return -1;
}

char* Server::GetIpFromSocket(int _socket)
{
	// create clientinfo storage
	SOCKADDR_IN client_info = { 0 };
	int addrsize = sizeof(client_info);
	// Populate it with ip e.t.c
	getpeername(_socket, (sockaddr*)&client_info, &addrsize);
	// Return its ip as a string
	return inet_ntoa(client_info.sin_addr);
}
