#pragma once

#define INCLUDE_SYS_SOCKETS
#include "base.hpp"
#include "IPAddress.hpp"
#include <thread>

namespace vsntwl {
	class Client {
	private:
		SOCKET client_socket;

		unsigned short port;
		InetProtocol inet_protocol;
		IPAddress4 address;

		std::thread client_thread;
	public:
		Client();
		~Client();

		void setInetProtocol(InetProtocol protocol);
		InetProtocol getInetProtocol() const;

		ClientConnectResult Connect(IPAddress4 address, unsigned short port);
		void disconnect();

		bool sendData(const char* data, unsigned int size);
	};
}