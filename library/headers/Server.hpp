#pragma once

#define INCLUDE_SYS_SOCKETS
#include "base.hpp"
#include <thread>
#include <mutex>
#include <map>
#include <functional>
#include <IPAddress.hpp>
#include "ConnectedClient.hpp"

namespace vsntwl {
	typedef std::function<void(ConnectedClient*, unsigned int)> client_conn_function;
	typedef std::function<void(ConnectedClient*, unsigned int, char*, unsigned int)> server_receive_function;
	typedef std::function<void(IPAddress4&, unsigned short, char*, unsigned int)> server_udp_receive_function;

	class Server {
	protected:
		unsigned short port;
		unsigned int max_connections;

		ServerStatus status;

		SOCKET server_socket;

		std::map<unsigned int, ConnectedClient*> clients;
		char* buffer;

		client_conn_function client_connect_handler;
		server_receive_function client_receive_handler;
		client_conn_function client_disconnect_handler;

		void disable_socket_blocking();

		std::pair<unsigned int, ConnectedClient*> getConnectionByAddress(const IPAddress4& address, unsigned short port);
	public:

		Server();
		virtual ~Server();
		//set server port
		void setPort(unsigned short port);
		//get server port
		unsigned short getPort() const;
		//set max client connections count
		void setMaxConnections(unsigned int max_connections);
		//get max clients connections
		unsigned int getMaxConnections() const;
		//get inet protocol, that set to this server
		virtual InetProtocol getInetProtocol() const = 0;
		//get current server status
		ServerStatus getStatus() const;
		//get all connected clients
		const std::map<unsigned int, ConnectedClient*>& getClients() const;
		//try to start server
		virtual ServerStartResult start() = 0;
		//disconnect all clients and stop server
		virtual void stop() = 0;
		//disconnect client by its id
		virtual void disconnect(unsigned int id) = 0;
		//handler for client connected event
		void setClientConnectedHandler(client_conn_function const& handler);
		void setClientDataReceiveHandler(server_receive_function const& handler);
		void setClientDisconnectedHandler(client_conn_function const& handler);
		//send data to all connected clients
		void sendAll(const char* data, unsigned int size);
		//send data to specified client, locking mutex
		void sendClient(unsigned int client_id, const char* data, unsigned int size);
		//send data to specified client, don't lock mutex
		int sendClientNoLock(unsigned int client_id, const char* data, unsigned int size);
	};
}