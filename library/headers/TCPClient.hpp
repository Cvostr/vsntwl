#pragma once

#include "Client.hpp"

namespace vsntwl {

	class TCPClient : public Client {
	private:
		std::thread* client_thread;

		void client_threaded_loop();
		void client_tcp_function();
		//sends just raw data without prefixes
		int sendPacket(const char* data, unsigned int size);
	public:
		TCPClient();
		~TCPClient();
		//get inet protocol, that set to this client
		InetProtocol getInetProtocol() const;
		//connect to server
		ClientConnectResult Connect(IPAddress4 address, unsigned short port);
		//disconnect from server
		void disconnect();
		//Send data to server
		bool sendData(const char* data, unsigned int size);
	};
}