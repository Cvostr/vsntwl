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
	protected:
		SOCKET client_socket;

		unsigned short port;
		IPAddress4 address;

		ClientStatus status;
		char* buffer;
		PacketStats packet_stats;
		std::thread client_thread;

		client_receive_function receive_handler;
		client_disconnect_function disconnect_handler;

		void disable_blocking();
	public:
		Client();
		virtual ~Client();
		//get inet protocol, that set to this client
		virtual InetProtocol getInetProtocol() const = 0;
		//get current client status
		ClientStatus getStatus() const;
		//get packets stats
		const PacketStats& GetPacketStats() const;
		//connect to server
		virtual ClientConnectResult Connect(IPAddress4 address, unsigned short port) = 0;
		//disconnect from server
		virtual void disconnect() {}
		//set data receive event handler
		void setDataReceivedHandler(client_receive_function const& handler);
		void setDisconnectHandler(client_disconnect_function const& handler);
	};
}