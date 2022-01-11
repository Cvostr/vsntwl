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
}