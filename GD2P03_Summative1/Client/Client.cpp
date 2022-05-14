#include "Client.h"

Client::Client()
{
	InitWSA();
	Init();
}

Client::~Client()
{
	for (auto& thread : m_ThreadPool)
	{
		if (thread.joinable())
			thread.join();
	}
	m_ThreadPool.clear();
	closesocket(m_ClientSocket);
	WSACleanup();
}

void Client::Init()
{
	m_ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_ClientSocket == -1)
	{
		printf("Error: Socket() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}

	printf("Enter Server IP: ");
	std::cin >> m_ServerIP;

	ConnectToServer(m_ServerIP, 5000);
	if (m_IsConnected)
	{
		m_ThreadPool.emplace_back(std::thread(&Client::RecieveFromServer, this));
		m_ThreadPool.emplace_back(std::thread(&Client::SendToServer, this));
	}
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
		status = recv(m_ClientSocket, m_InBuffer, BUFFER_SIZE, 0);

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
			if (sizeof(m_InBuffer) / sizeof(char) > status)
				m_InBuffer[status] = '\0';

			printf("%s\n", m_InBuffer);
		}
	} while (status > 0);
}

void Client::SendToServer()
{
	auto status{0};
	bool finishedMessage{ false };
	std::string message{};
	do
	{
		if (m_IsConnected)
		{
			std::cin >> message;
			if (message.length() >= BUFFER_SIZE - 1)
			{
				message.erase(BUFFER_SIZE - 1);
			}
			message.shrink_to_fit();
			strcpy_s(m_OutBuffer, message.c_str());

			if (ReconnectionCheck(std::move(m_OutBuffer[0])))
				return;

			status = send(m_ClientSocket, m_OutBuffer, strlen(m_OutBuffer), 0);
			if (status == -1)
			{
				printf("Send Error!\n");
				return;
			}
			if ((std::string)m_OutBuffer == "QUIT")
			{
				Sleep(1000);
				exit(0);
			}
		}
	} while (true);
}

void Client::ConnectToServer(std::string _ip, u_short&& _port)
{
	sockaddr_in receiverAddr;
	receiverAddr.sin_family = AF_INET;
	receiverAddr.sin_port = htons(_port);
	receiverAddr.sin_addr.s_addr = inet_addr(_ip.data());
	
	printf("Attempting Connection To %s\n", _ip.c_str());
	auto status = connect(m_ClientSocket, (sockaddr*)&receiverAddr, sizeof(sockaddr));
	if (status == -1)
	{
		printf("Connection To %s Failed!\n", _ip.data());
	}
	else
	{
		m_IsConnected = true;
		
	}
}

char* Client::GetIpFromSocket(int&& _socket)
{
	SOCKADDR_IN client_info = { 0 };
	int addrsize = sizeof(client_info);
	getpeername(_socket, (sockaddr*)&client_info, &addrsize);
	return inet_ntoa(client_info.sin_addr);
}

bool Client::ReconnectionCheck(char&& _input)
{
	if (!m_IsConnected)
	{
		if (_input == '1')
		{
			closesocket(m_ClientSocket);
			Init();
			return true;
		}
		else
		{
			printf("Closing Program...");
			Sleep(1000);
			exit(0);
		}
	}
	return false;
}
