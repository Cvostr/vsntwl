#pragma once

#define INCLUDE_SYS_SOCKETS
#include "base.hpp"
#include "IPAddress.hpp"
#include <thread>
#include <functional>

namespace vsntwl {

	typedef std::function<void(char*, unsigned int)> client_receive_function;

	class Client {
	private:
		SOCKET client_socket;

		unsigned short port;
		InetProtocol inet_protocol;
		IPAddress4 address;

		ClientStatus status;

		std::thread client_thread;
		client_receive_function receive_handler;

		char* buffer;

		void client_threaded_loop();
	public:
		Client();
		~Client();

		void setInetProtocol(InetProtocol protocol);
		InetProtocol getInetProtocol() const;

		ClientStatus getStatus() const;

		ClientConnectResult Connect(IPAddress4 address, unsigned short port);
		void disconnect();

		bool sendData(const char* data, unsigned int size);

		void setDataReceivedHandler(client_receive_function const& handler);
	};
}