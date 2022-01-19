#pragma once

#include "Server.hpp"

namespace vsntwl {
	class TCPServer : public Server {
	private:
		std::thread accept_thread;
		std::thread data_thread;
		std::mutex client_mutex;

		void accept_threaded_loop();
		void data_threaded_loop();

	public:

		TCPServer();
		~TCPServer();
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
		int sendClientNoLock(unsigned int client_id, const char* data, unsigned int size);
	};
}