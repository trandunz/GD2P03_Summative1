#include "Server.h"

Server::Server()
{
	InitWSA();

	m_Commands.emplace_back(Command{ "CAPITALIZE", [this](int _client,char* _buffer,bool _finished)
		{
			if (_finished)
			{
				m_SavedMesssages[_client].pop_back();
				std::string upperString{};
				for (auto& character : m_SavedMesssages[_client])
				{
					upperString += toupper(character);
				}
				SendMessageToClient(_client,upperString);
			}
			else
			{
				m_SavedMesssages[_client] += _buffer;
				m_SavedMesssages[_client] += '\n';
			}

		} });
	m_Commands.emplace_back(Command{ "PUT", [this](int _client,char* _buffer,bool _finished)
		{
			if (_finished)
			{
				m_SavedMesssages[_client].pop_back();
				SendMessageToClient(_client, "200 OK");
			}
			else if (m_SavedMesssages[_client].length() < BUFFER_SIZE)
			{
				m_SavedMesssages[_client] += _buffer;
				m_SavedMesssages[_client] += '\n';
			}
		} });
	m_Commands.emplace_back(Command{ "GET", [this](int _client,char* _buffer,bool _finished)
		{
			SendMessageToClient(_client,m_SavedMesssages[_client]);
		}, true });
	m_Commands.emplace_back(Command{ "QUIT", [this](int _client,char* _buffer,bool _finished)
		{
			Sleep(1000);
			exit(0);
		}, true });

	Init(5000);
}

Server::~Server()
{
	for (auto& thread : m_ThreadPool)
	{
		if (thread.joinable())
			thread.join();
	}
	m_ThreadPool.clear();

	for (auto& socket : m_SavedMesssages)
	{
		closesocket(socket.first);
	}
	closesocket(m_ServerSocket);
	WSACleanup();
}

void Server::Init(int _port)
{
	m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_ServerSocket == -1)
	{
		printf("Error: Socket() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}

	// Uniform Initialization
	struct sockaddr_in serverAddr { AF_INET , htons(_port), INADDR_ANY };

	int status{};
	status = bind(m_ServerSocket, (sockaddr*)&serverAddr, sizeof(sockaddr));
	if (status == 1)
	{
		printf("Error: Bind() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}

	Listen();
	Accept();
}

void Server::Listen()
{
	printf("Listening for connections...\n");
	int status{};
	status = listen(m_ServerSocket, SOMAXCONN);
	if (status == -1)
	{
		printf("Error: Listen() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}

}

void Server::Accept()
{
	struct sockaddr_in senderAddr;
	int length = sizeof(senderAddr);
	auto client{ 0 };
	while (client = accept(m_ServerSocket, (sockaddr*)&senderAddr, &length))
	{
		if (client == INVALID_SOCKET)
		{
			printf("Invalid Client Socket\n");
			continue;
		}
		m_SavedMesssages.insert(std::make_pair(client, ""));
		m_ThreadPool.emplace_back(std::thread(&Server::SendAndRecieve, this, client));
	}
}

void Server::SendAndRecieve(int _client)
{
	printf("Server: Connection From Client %s\n", GetIpFromSocket(_client));
	SendMessageToClient(_client, "Server is ready...");

	char buffer[BUFFER_SIZE];
	auto status{0};
	bool executingCommand{ false };
	int activeCommandIndex{ -1 };
	while (status = recv(_client, buffer, BUFFER_SIZE, 0))
	{
		if (sizeof(buffer) / sizeof(char) > status)
			buffer[status] = '\0';

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
		else
		{
			std::string incomingMessage{ buffer };
			if (!executingCommand)
			{
				activeCommandIndex = CheckValidCommand(_client, buffer, executingCommand, incomingMessage);
			}
			else if (executingCommand && activeCommandIndex != -1)
			{
				if (incomingMessage == ".")
				{
					m_Commands[activeCommandIndex].function(_client, buffer, true);

					executingCommand = false;
					activeCommandIndex = -1;
				}
				else
				{
					m_Commands[activeCommandIndex].function(_client, buffer, false);
				}
			}
		}


	}
	
}

void Server::SendMessageToClient(int _client, std::string _message)
{
	send(_client, _message.data(), _message.length(), 0);
}

void Server::PrintIPSentCommand(int _client, std::string _incomingCommand)
{
	SOCKADDR_IN client_info{ 0 };
	int size = sizeof(client_info);
	getpeername(_client, (sockaddr*)&client_info, &size);
	std::cout << inet_ntoa(client_info.sin_addr) << " sends " << _incomingCommand << std::endl;
}

int Server::CheckValidCommand(int _client, char* _buffer,bool& _executingCommand, std::string _incomingCommand)
{
	int i{ 0 };
	for (auto& command : m_Commands)
	{
		if (_incomingCommand == command.name)
		{
			SendMessageToClient(_client,"200 OK");
			PrintIPSentCommand(_client,_incomingCommand);

			if (command.immediate)
			{
				command.function(_client, _buffer,true);
				_executingCommand = false;
			}
			else
			{
				m_SavedMesssages[_client].clear();
				_executingCommand = true;
			}
			return i;
		}
		i++;
	}
	SendMessageToClient(_client,"400 Command not valid.");
	return -1;
}

char* Server::GetIpFromSocket(int _socket)
{
	SOCKADDR_IN client_info = { 0 };
	int addrsize = sizeof(client_info);
	getpeername(_socket, (sockaddr*)&client_info, &addrsize);
	return inet_ntoa(client_info.sin_addr);
}
