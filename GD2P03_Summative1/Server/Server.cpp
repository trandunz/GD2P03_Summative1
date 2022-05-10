#include "Server.h"

Server::Server()
{
	InitWSA();
	m_Commands.emplace_back(Command{ "CAPITALIZE", [this](int _client,bool _finished)
		{
			if (_finished)
			{
				savedMessage.pop_back();
				std::string upperString{};
				for (auto& character : savedMessage)
				{
					upperString += toupper(character);
				}
				SendMessageToClient(_client,upperString);
			}
			else
			{
				savedMessage += m_Buffer;
				savedMessage += '\n';
			}

		} });
	m_Commands.emplace_back(Command{ "PUT", [this](int _client,bool _finished)
		{
			if (_finished)
			{
				savedMessage.pop_back();
			}
			else
			{
				savedMessage += m_Buffer;
				savedMessage += '\n';
			}
		} });
	m_Commands.emplace_back(Command{ "GET", [this](int _client,bool _finished)
		{
			if (_finished)
			{
				SendMessageToClient(_client,savedMessage);
			}
			else
			{
				SendMessageToClient(_client,savedMessage);
			}
		}, true });
	m_Commands.emplace_back(Command{ "QUIT", [this](int _client,bool _finished)
		{
			if (_finished)
			{
				SendMessageToClient(_client,"QUIT");
				exit(0);
			}
			else
			{
				SendMessageToClient(_client,"QUIT");
				exit(0);
			}
		}, true });
	Init(5001);
}

Server::~Server()
{
	for (auto& thread : m_ThreadPool)
	{
		if (thread.joinable())
			thread.join();
	}
	m_ThreadPool.clear();
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

	m_Status = bind(m_ServerSocket, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr));
	if (m_Status == 1)
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
	m_Status = listen(m_ServerSocket, 5);
	if (m_Status == -1)
	{
		printf("Error: Listen() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}

}

void Server::Accept()
{
	struct sockaddr_in senderAddr;
	int length = sizeof(senderAddr);
	int client{ 0 };
	while (client = accept(m_ServerSocket, (struct sockaddr*)&senderAddr, &length))
	{
		if (client == INVALID_SOCKET)
		{
			printf("Invalid Client Socket\n");
			continue;
		}
		m_ThreadPool.emplace_back(std::thread(&Server::SendAndRecieve, this, client));
	}
}

void Server::SendAndRecieve(int _client)
{
	printf("A Connection from IP has been accepted\n");

	while (m_Status = recv(_client, m_Buffer, BUFFER_SIZE, 0))
	{
		if (sizeof(m_Buffer) / sizeof(char) > m_Status)
			m_Buffer[m_Status] = '\0';

		if (m_Status == 0)
		{
			printf("Error: Client Disconnected\n");
			return;
		}
		else if (m_Status == -1)
		{
			int error = WSAGetLastError();
			printf("Error: recv() failed. Code: %d\n", error);
			return;
		}
		else
		{
			std::string incomingMessage{ m_Buffer };
			if (!m_IsExecutingCommand)
			{
				m_ActiveCommandIndex = CheckValidCommand(_client,incomingMessage);
			}
			else if (m_IsExecutingCommand && m_ActiveCommandIndex != -1)
			{
				if (incomingMessage == ".")
				{
					m_Commands[m_ActiveCommandIndex].function(_client,true);

					m_IsExecutingCommand = false;
					m_ActiveCommandIndex = -1;
				}
				else
				{
					m_Commands[m_ActiveCommandIndex].function(_client,false);
				}
			}
		}
	}
	
}

void Server::SendMessageToClient(int _client, std::string _message)
{
	strcpy_s(m_Buffer, _message.c_str());
	send(_client, m_Buffer, strlen(m_Buffer), 0);
}

void Server::PrintIPSentCommand(int _client, std::string _incomingCommand)
{
	SOCKADDR_IN client_info{ 0 };
	int size = sizeof(client_info);
	getpeername(_client, (struct sockaddr*)&client_info, &size);
	std::cout << inet_ntoa(client_info.sin_addr) << " sends " << _incomingCommand << std::endl;
}

int Server::CheckValidCommand(int _client,std::string _incomingCommand)
{
	int i = 0;
	for (auto& command : m_Commands)
	{
		if (_incomingCommand == command.name)
		{
			SendMessageToClient(_client,"200 OK");
			PrintIPSentCommand(_client,_incomingCommand);

			if (command.immediate)
			{
				command.function(_client,true);
				m_IsExecutingCommand = false;
			}
			else
			{
				savedMessage.clear();
				m_IsExecutingCommand = true;
			}
			return i;
		}
		i++;
	}
	SendMessageToClient(_client,"400 Command not valid.");
	return -1;
}
