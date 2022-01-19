#include <Server.hpp>
#include <chrono>
#include <vector>
#include <Socket.hpp>

using namespace vsntwl;

Server::Server() {
	port = 34759;
	status = SERVER_STATUS_DOWN;
	server_socket = 0;
	max_connections = 0; // no limit

	buffer = new char[DEFAULT_BUFLEN];
}
Server::~Server() {
	delete[] buffer;
}

void Server::setPort(unsigned short port) {
	this->port = port;
}
unsigned short Server::getPort() const {
	return port;
}
void Server::setMaxConnections(unsigned int max_connections) {
	this->max_connections = max_connections;
}
unsigned int Server::getMaxConnections() const {
	return max_connections;
}
ServerStatus Server::getStatus() const {
	return status;
}
const std::map<unsigned int, ConnectedClient*>& Server::getClients() const {
	return clients;
}
void Server::setClientConnectedHandler(client_conn_function const& handler) {
	this->client_connect_handler = handler;
}
void Server::setClientDataReceiveHandler(server_receive_function const& handler) {
	this->client_receive_handler = handler;
}
void Server::setClientDisconnectedHandler(client_conn_function const& handler) {
	this->client_disconnect_handler = handler;
}
void Server::disable_socket_blocking() {
	DisableBlocking(server_socket);
}

std::pair<unsigned int, ConnectedClient*> Server::getConnectionByAddress(const IPAddress4& address, unsigned short port) {
	for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
		auto& client_pair = *it;
	
		if (client_pair.second->getPort() == port && client_pair.second->getIP() == address) {
			return client_pair;
		}
	}

	return std::make_pair(0, nullptr);
}