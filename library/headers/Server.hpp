#pragma once

#define INCLUDE_SYS_SOCKETS
#include "base.hpp"
#include <thread>
#include <mutex>
#include <map>
#include <functional>
#include <IPAddress.hpp>

namespace vsntwl {
	class ConnectedClient {
	private:
		SOCKET client_socket;
		IPAddress4 client_ip;
	public:

		SOCKET GetSocket() const {
			return client_socket;
		}

		ConnectedClient(SOCKET sock, IPAddress4 ip) {
			client_socket = sock;
			client_ip = ip;
		}
	};

	typedef std::function<void(ConnectedClient*, unsigned int)> client_conn_function;
	typedef std::function<void(ConnectedClient*, unsigned int, char*, unsigned int)> server_receive_function;

	class Server {
	private:
		unsigned short port;
		InetProtocol inet_protocol;
		unsigned int max_connections;

		ServerStatus status;

		SOCKET server_socket;

		std::thread accept_thread;
		std::thread data_thread;
		std::mutex client_mutex;

		std::map<unsigned int, ConnectedClient*> clients;

		char* buffer;

		client_conn_function client_connect_handler;
		server_receive_function client_receive_handler;
		client_conn_function client_disconnect_handler;

		void accept_threaded_loop();
		void data_threaded_loop();
	public:

		Server();
		~Server();
		//set server port
		void setPort(unsigned short port);
		//get server port
		unsigned short getPort() const;

		void setMaxConnections(unsigned int max_connections);

		unsigned int getMaxConnections() const;
		//set internet protocol for this server (TCP, UDP)
		void setInetProtocol(InetProtocol protocol);
		InetProtocol getInetProtocol() const;
		//get current server status
		ServerStatus getStatus() const;
		//get all connected clients
		const std::map<unsigned int, ConnectedClient*>& getClients() const;
		//try to start server
		ServerStartResult start();
		//disconnect all clients and stop server
		void stop();
		//disconnect client by its id
		void disconnect(unsigned int id);

		void setClientConnectedHandler(client_conn_function const& handler);
		void setClientDataReceiveHandler(server_receive_function const& handler);
		void setClientDisconnectedHandler(client_conn_function const& handler);
		//send data to all connected clients
		void sendAll(const char* data, unsigned int size);
		//send data to specified client
		void sendClient(unsigned int client_id, const char* data, unsigned int size);
	};
}