#pragma once
#include <string>
#include <stdio.h>
#include <winsock.h>


constexpr int BUFFER_SIZE = 100;

inline void InitWSA()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(1, 1);

	int nRc = WSAStartup(wVersionRequested, &wsaData);

	if (nRc != 0)
	{
		fprintf(stderr, "Error: WSAStartup Failed\n");
		exit(-1);
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		fprintf(stderr, "Error: Version is not available\n");
		exit(-1);
	}
}

