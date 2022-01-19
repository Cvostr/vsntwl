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
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define WSAECONNRESET 104
#endif

#define DEFAULT_BUFLEN 1500

#define UDP_PREFIX_USER_CONNECTION 0x1
#define UDP_PREFIX_USER_DISCONNECTION 0x2
#define UDP_PREFIX_ACK 0x3
#define UDP_PREFIX_SUCCESS 0x4
#define UDP_PREFIX_FAIL 0x5
#define UDP_PREFIX_PACKET 0x6
#define UDP_PREFIX_PACKET_RECEIVED 0x7

enum InetProtocol {
	INET_PROTOCOL_TCP,
	INET_PROTOCOL_UDP
};

enum ServerStatus {
	SERVER_STATUS_DOWN,
	SERVER_STATUS_STARTING,
	SERVER_STATUS_UP
};

enum ClientStatus {
	CLIENT_STATUS_DISCONNECTED,
	CLIENT_STATUS_CONNECTED
};

enum ServerStartResult {
	SERVER_START_SUCCESSFUL = 0,
	SERVER_ERROR_ON_INIT,
	SERVER_ERROR_ON_BIND,
	SERVER_ERROR_ON_LISTEN,
};

enum ClientConnectResult {
	CLIENT_CONNECTED,
	CLIENT_ERROR_SOCKET,
	CLIENT_ERROR_CONNECTION
};

enum DisconnectionType {
	DISCONNECTION_TYPE_MANUAL,
	DISCONNECTION_TYPE_FORCIBLY
};

struct PacketStats {
	unsigned int received_packets;
	unsigned int sent_packets;

	PacketStats() : received_packets(0), sent_packets(0) {}
};

struct UDP_ReliablePacket {
	unsigned int pk_num;
	unsigned int pk_size;
	unsigned int last_try_time;
	char* pk_data;
};

#define vsntwl_free(x) delete x; x = nullptr;

int ConvertProtocol(InetProtocol protocol);
unsigned int get_random_value();
unsigned int get_random_value(unsigned int min, unsigned int max);