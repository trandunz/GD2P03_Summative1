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
	struct sockaddr_in receiverAddr;
	receiverAddr.sin_family = AF_INET;
	receiverAddr.sin_port = htons(5001);
	receiverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	m_Status = connect(m_ClientSocket, (struct sockaddr*)&receiverAddr, sizeof(struct sockaddr));
	if (m_Status == -1)
	{
		printf("Error: Connect() failed. Code: %d\n", WSAGetLastError());
		exit(-1);
	}
}

Client::~Client()
{
	closesocket(m_ClientSocket);
	WSACleanup();
}

void Client::Update()
{
	// Clear Buffer
	memset(m_Buffer, 0, sizeof m_Buffer);

	m_Status = recv(m_ClientSocket, m_Buffer, BUFFER_SIZE, 0);
	if (m_Status != -1)
	{
		if ((std::string)m_Buffer == "QUIT")
		{
			exit(0);
		}
		if ((std::string)m_Buffer != " ")
		{
			m_Buffer[m_Status] = '\0';
			printf("%s\n", m_Buffer);
		}
	}

	gets_s(m_Buffer);
	m_Status = send(m_ClientSocket, m_Buffer, strlen(m_Buffer), 0);
	if (m_Status == -1)
	{
		printf("Error: Send() failed. Code: %d\n", WSAGetLastError());
		return;
	}
}
