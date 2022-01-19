#pragma once

#include "Client.hpp"
#include <vector>

namespace vsntwl {

	class UDPClient : public Client {
	private:
		std::thread* client_thread;
		sockaddr_in server_addr;
		std::vector<UDP_ReliablePacket*> reliable_packets;

		void client_threaded_loop();
		void client_udp_function();
		//sends just raw data without prefixes
		int sendPacket(const char* data, unsigned int size);

		unsigned int sendReliablePacket(const char* data, unsigned int size)
	public:
		UDPClient();
		~UDPClient();
		//get inet protocol, that set to this client
		InetProtocol getInetProtocol() const;
		//connect to server
		ClientConnectResult Connect(IPAddress4 address, unsigned short port);
		//disconnect from server
		void disconnect();
		//Send data to server
		//if UDP, this method is INRELIABLE
		bool sendData(const char* data, unsigned int size);
	};
}