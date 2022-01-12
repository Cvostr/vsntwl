#pragma once

#define INCLUDE_SYS_SOCKETS
#include "base.hpp"
#include "IPAddress.hpp"
#include <thread>
#include <functional>

namespace vsntwl {

	typedef std::function<void(char*, unsigned int)> client_receive_function;
	typedef std::function<void()> client_disconnect_function;

	class Client {
	private:
		SOCKET client_socket;

		unsigned short port;
		InetProtocol inet_protocol;
		IPAddress4 address;

		ClientStatus status;

		std::thread* client_thread;
		client_receive_function receive_handler;
		client_disconnect_function disconnect_handler;

		char* buffer;

		void client_threaded_loop();

		void disable_tcp_blocking();

		void client_tcp_function();
		void client_udp_function();
	public:
		Client();
		~Client();
		//set internet protocol for this client (TCP, UDP)
		void setInetProtocol(InetProtocol protocol);
		//get inet protocol, that set to this client
		InetProtocol getInetProtocol() const;
		//get current client status
		ClientStatus getStatus() const;
		//connect to server
		ClientConnectResult Connect(IPAddress4 address, unsigned short port);
		//disconnect from server
		void disconnect();
		//Send data to server
		bool sendData(const char* data, unsigned int size);

		void setDataReceivedHandler(client_receive_function const& handler);
		void setDisconnectHandler(client_disconnect_function const& handler);
	};
}