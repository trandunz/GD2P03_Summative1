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

	/*auto error{0};
	auto status{0};
	u_short preferedPort = 7000;
	do
	{
		sockaddr_in clientAddr { AF_INET, htons(preferedPort), INADDR_ANY };

		status = bind(m_ClientSocket, (sockaddr*)&clientAddr, sizeof(sockaddr));

		if (status == -1)
		{
			error = WSAGetLastError();
			if (error == WSAEADDRINUSE)
			{
				printf("Error: Port %i Already In Use. Reattempting With Port %i\n", preferedPort, preferedPort + 1);
			}
			else if (error == WSAEINVAL)
			{
				printf("Error: Invalid Bind Argument. Multiple Instances Running On Same Machine?\n");
			}
			else
			{
				printf("Error: Bind() failed. Code: %d\n", WSAGetLastError());
			}
			preferedPort++;
		}
		
	} while (error == WSAEADDRINUSE);*/


	std::string ip;
	printf("Enter Server IP: ");
	std::cin >> ip;

	int port;
	printf("Enter Server Port: ");
	std::cin >> port;

	ConnectToServer(ip, std::move(port));
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
			printf("Client: Disconnection From Server %s\n", GetIpFromSocket(m_ClientSocket));
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
	do
	{
		if (m_IsConnected)
		{
			gets_s(m_OutBuffer);

			if (ReconnectionCheck(m_OutBuffer[0]))
				return;
		}

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

char* Client::GetIpFromSocket(int _socket)
{
	SOCKADDR_IN client_info = { 0 };
	int addrsize = sizeof(client_info);
	getpeername(_socket, (sockaddr*)&client_info, &addrsize);
	return inet_ntoa(client_info.sin_addr);
}

bool Client::ReconnectionCheck(char _input)
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
