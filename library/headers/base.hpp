#pragma once

#ifdef INCLUDE_SYS_SOCKETS

#ifdef _WIN32
#include <WinSock2.h>
#include <mstcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#endif

#ifdef __linux__
typedef int SOCKET;
#endif

#define DEFAULT_BUFLEN 1500

enum InetProtocol {
	INET_PROTOCOL_TCP,
	INET_PROTOCOL_UDP
};

enum ServerStatus {
	SERVER_STATUS_DOWN,
	SERVER_STATUS_STARTING,
	SERVER_STATUS_UP
};

enum ServerStartResult {
	SERVER_START_SUCCESSFUL = 0,
	SERVER_ERROR_ON_INIT,
	SERVER_ERROR_ON_BIND,
	SERVER_ERROR_ON_LISTEN,
};

enum ClientConnectResult {
	CLIENT_CONNECTED,
	CLIENT_CONNECTION_FAILED
};

int ConvertProtocol(InetProtocol protocol);