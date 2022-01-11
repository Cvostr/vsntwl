#pragma once

#define INCLUDE_SYS_SOCKETS
#include "base.hpp"
#include "IPAddress.hpp"

namespace vsntwl {
	class Socket {
	private:
		SOCKET socket;
		bool opened;
	public:

		Socket();
		~Socket();

		int Create(InetProtocol protocol);
		int Bind(const IPAddress4& address, unsigned short port);
		int Listen();
		int Accept();

		int Connect(IPAddress4& address, unsigned short port);

		int Recv(const Socket& sock, char* data, unsigned int& size);
		int Send(const Socket& sock, char* data, unsigned int& size);

		int Close();
	};
}