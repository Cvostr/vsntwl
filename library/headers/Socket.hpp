#pragma once

#define INCLUDE_SYS_SOCKETS
#include "base.hpp"
#include "IPAddress.hpp"

namespace vsntwl {
	int CloseSocket(SOCKET socket);
	int GetLastSockErrCode();
	int DisableBlocking(SOCKET socket);
	void FillInaddrStruct(const IPAddress4& address, unsigned short port, sockaddr_in& out);
	void FillInaddrStruct(unsigned short port, sockaddr_in& out);
	unsigned int GetAddressInteger(const sockaddr_in& in);
	SOCKET AcceptSocket(SOCKET socket, sockaddr_in& in);
	//UDP
	int RecvFrom(SOCKET socket, char* buffer, unsigned int size, sockaddr_in& sender);
	int SendTo(SOCKET socket, const char* buffer, unsigned int size, const sockaddr_in& dest);
}