#include "Server.h"

Server::Server()
{
	InitWSA();
	Init(5001);
	m_Commands.emplace_back(std::make_pair("CAPITALIZE", [this](bool _finished) 
		{
			if (_finished)
			{
				savedMessage.pop_back();
				std::string upperString{};
				for (auto& character : savedMessage)
				{
					upperString += toupper(character);
				}
				SendMessageToClient(upperString);
			}
			else
			{
				savedMessage += m_Buffer;
				SendMessageToClient(" ");
				savedMessage += '\n';
			}
			
		}));
	m_Commands.emplace_back(std::make_pair("PUT", [this](bool _finished)
		{
			if (_finished)
			{
				savedMessage.pop_back();
				SendMessageToClient(" ");
			}
			else
			{
				savedMessage += m_Buffer;
				SendMessageToClient(" ");
				savedMessage += '\n';
			}
		}));
	m_Commands.emplace_back(std::make_pair("GET", [this](bool _finished)
		{
			if (_finished)
			{
				SendMessageToClient(savedMessage);
			}
			else
			{
				SendMessageToClient(savedMessage);
			}
		}));
	m_Commands.emplace_back(std::make_pair("QUIT", [this](bool _finished)
		{
			if (_finished)
			{
				SendMessageToClient("QUIT");
				exit(0);
			}
			else
			{
				SendMessageToClient("QUIT");
				exit(0);
			}
		}));
}

Server::~Server()
{
	closesocket(m_ListenerSocket);
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

void Server::Update()
{

	if (m_IsConnected)
	{
		memset(m_Buffer, 0, sizeof m_Buffer);

		printf("Waiting for a message...\n");
		m_Status = recv(m_ListenerSocket, m_Buffer, BUFFER_SIZE, 0);

		if (m_Status == 0)
		{
			printf("Error: Client Disconnected\n");
			m_IsConnected = false;
		}
		else if (m_Status == -1)
		{
			int error = WSAGetLastError();
			printf("Error: recv() failed. Code: %d\n", error);
			return;
		}

		std::string incomingMessage{ m_Buffer };
		if (!m_IsExecutingCommand)
		{
			m_ActiveCommandIndex = CheckValidCommand(incomingMessage);
		}
		else if (m_IsExecutingCommand && m_ActiveCommandIndex != -1)
		{
			if (incomingMessage == ".")
			{
				m_Commands[m_ActiveCommandIndex].second(true);

				m_IsExecutingCommand = false;
				m_ActiveCommandIndex = -1;
			}
			else
			{
				m_Commands[m_ActiveCommandIndex].second(false);
			}
		}
	}
	else
	{
		Listen();
		Accept();
	}
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
	m_ListenerSocket = accept(m_ServerSocket, (struct sockaddr*)&senderAddr, &length);
	if (m_ListenerSocket == 0)
	{
		printf("Client Disconnected...\n");
	}
	if (m_ListenerSocket == -1)
	{
		printf("Error: Accept() failed. Code: %d\n", WSAGetLastError());
		printf("Accepting next connection...\n\n");
	}
	printf("A Connection from %s has been accepted\n", inet_ntoa(senderAddr.sin_addr));
	m_IsConnected = true;

	SendMessageToClient("Server is Ready...");
}

void Server::SendMessageToClient(std::string _message)
{
	strcpy_s(m_Buffer, _message.c_str());
	send(m_ListenerSocket, m_Buffer, strlen(m_Buffer), 0);
}

void Server::PrintIPSentCommand(std::string _incomingCommand)
{
	SOCKADDR_IN client_info{ 0 };
	int size = sizeof(client_info);
	getpeername(m_ListenerSocket, (struct sockaddr*)&client_info, &size);
	std::cout << inet_ntoa(client_info.sin_addr) << " sends " << _incomingCommand << std::endl;
}

int Server::CheckValidCommand(std::string _incomingCommand)
{
	int i = 0;
	for (auto& command : m_Commands)
	{
		if (_incomingCommand == command.first)
		{
			SendMessageToClient("200 OK");
			PrintIPSentCommand(_incomingCommand);
			m_IsExecutingCommand = true;
			return i;
		}
		i++;
	}
	SendMessageToClient("400 Command not valid.");
	return -1;
}
