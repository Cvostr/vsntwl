#pragma once

#include "Server.hpp"

namespace vsntwl {
	class UDPServer : public Server {
	private:
		std::thread clients_check_thread;
		std::thread data_thread;
		std::mutex client_mutex;
		char* temp_send_buffer;

		void data_threaded_loop();
		void clients_check_loop();

	public:

		UDPServer();
		~UDPServer();
		//get inet protocol, that set to this server
		InetProtocol getInetProtocol() const;
		//get all connected clients
		const std::map<unsigned int, ConnectedClient*>& getClients() const;
		//try to start server
		ServerStartResult start();
		//disconnect all clients and stop server
		void stop();
		//disconnect client by its id
		void disconnect(unsigned int id);
		//send data to all connected clients
		void sendAll(const char* data, unsigned int size);
		//send data to specified client, locking mutex
		void sendClient(unsigned int client_id, const char* data, unsigned int size);
		//send data to specified client, don't lock mutex
		int sendPacketClientNoLock(unsigned int client_id, const char* data, unsigned int size);
		int sendClientNoLock(unsigned int client_id, const char* data, unsigned int size);
	};
}