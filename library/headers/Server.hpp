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

		void setPort(unsigned short port);
		unsigned short getPort() const;

		void setInetProtocol(InetProtocol protocol);
		InetProtocol getInetProtocol() const;

		ServerStatus getStatus() const;

		const std::map<unsigned int, ConnectedClient*>& getClients() const;

		ServerStartResult start();
		void stop();

		void disconnect(unsigned int id);

		void setClientConnectedHandler(client_conn_function const& handler);
		void setClientDataReceiveHandler(server_receive_function const& handler);
		void setClientDisconnectedHandler(client_conn_function const& handler);

		void sendAll(const char* data, unsigned int size);
	};
}