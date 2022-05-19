#pragma once
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <winsock.h>
#include <map>

// Holds functionality for a command
using CommandFunction = std::function<void(int _client, char* _buffer, bool _finished)>;

// Maximum Buffer Size
constexpr int BUFFER_SIZE = 100;

// Contains Command Data.
// string: command name
// CommandFunction: funcionality
// bool: is this command to be run immediatly?
struct Command
{
	std::string name{};
	CommandFunction function{nullptr};
	bool immediate{ false };
};

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
	// Check if WSAStartup failed
	if (status != 0)
	{
		fprintf(stderr, "Error: WSAStartup Failed\n");
		exit(-1);
	}
}

