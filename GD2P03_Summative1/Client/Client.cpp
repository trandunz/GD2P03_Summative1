#include "Client.h"

Client::Client()
{
	InitWSA();

	m_ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_ClientSocket == -1)
	{
		printf("Error: Socket() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}

	struct sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(7001);
	clientAddr.sin_addr.s_addr = INADDR_ANY;

	m_Status = bind(m_ClientSocket, (struct sockaddr*)&clientAddr, sizeof(struct sockaddr));
	if (m_Status == -1)
	{
		printf("Error: Bind() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}
	
	ConnectToServer("127.0.0.1");
	m_ThreadPool.emplace_back(std::thread(&Client::RecieveFromServer, this));
	m_ThreadPool.emplace_back(std::thread(&Client::SendToServer, this));
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

void Client::RecieveFromServer()
{
	int status{ 0 };
	do 
	{
		std::lock_guard<std::mutex>lk(g_mutex);
		g_cv.notify_one();

		status = recv(m_ClientSocket, m_InBuffer, BUFFER_SIZE, 0);

		if (sizeof(m_InBuffer) / sizeof(char) > status)
			m_InBuffer[status] = '\0';

		if (status == -1)
		{
			printf("recv Error!\n");
			printf("Press 1 to Retry, Press AnyKey to Exit Program\n");
			m_IsConnected = false;
			g_cv.notify_one();
			char retry{};
			retry = _getch();
			if (retry == 51)
				ConnectToServer("127.0.0.1");
			else
				exit(0);
			return;
		}
		else if (status == 0)
		{
			m_IsConnected = false;
			g_cv.notify_one();
			printf("Server Disconnected!\n");
			return;
		}
		
		printf("%s\n", m_InBuffer);
		g_cv.notify_one();
	} while (status > 0);
}

void Client::SendToServer()
{
	int status{ 0 };
	do
	{
		std::unique_lock<std::mutex> lk(g_mutex);
		g_cv.wait(lk, [this]()->bool {return m_IsConnected; });
		gets_s(m_OutBuffer);
		status = send(m_ClientSocket, m_OutBuffer, strlen(m_OutBuffer), 0);
	} 
	while (status > 0);
}

void Client::ConnectToServer(std::string _ip)
{
	int status{ 0 };

	struct sockaddr_in receiverAddr;
	receiverAddr.sin_family = AF_INET;
	receiverAddr.sin_port = htons(5001);
	receiverAddr.sin_addr.s_addr = inet_addr(_ip.data());
	
	status = connect(m_ClientSocket, (struct sockaddr*)&receiverAddr, sizeof(struct sockaddr));
	if (status == -1)
	{
		printf("Connection To %s Failed!", _ip.data());
		status = connect(m_ClientSocket, (struct sockaddr*)&receiverAddr, sizeof(struct sockaddr));
	}
	m_IsConnected = true;
	g_cv.notify_one();
}
