#pragma once
#include <string>
#include <stdio.h>
#include <winsock.h>
#include <vector>
#include <thread>
#include <iostream>
#include <conio.h>
#include <mutex>

// Maximum Buffer Size
constexpr int BUFFER_SIZE = 100;

/// <summary>
/// Initalizes WSA For Use Of Sockets
/// </summary>
inline void InitWSA()
{
	WSADATA wsaData;
	auto version = MAKEWORD(1, 1);
	// Start WSA with version 1.1
	auto status = WSAStartup(version, &wsaData);
	// Check for incorrect version
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		fprintf(stderr, "Error: Version is not available\n");
		exit(-1);
	}
	// Check for error
	if (status != 0)
	{
		fprintf(stderr, "Error: WSAStartup Failed\n");
		exit(-1);
	}
	
}

