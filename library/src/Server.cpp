#include <Server.hpp>
#include <chrono>
#include <vector>
#include <Socket.hpp>

using namespace vsntwl;

Server::Server() {
	port = 34759;
	inet_protocol = INET_PROTOCOL_TCP;
	status = SERVER_STATUS_DOWN;
	server_socket = 0;
	max_connections = 0; // no limit

	buffer = new char[DEFAULT_BUFLEN];
}
Server::~Server() {
	delete[] buffer;
	stop();
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
void Server::setInetProtocol(InetProtocol protocol) {
	inet_protocol = protocol;
}
InetProtocol Server::getInetProtocol() const {
	return inet_protocol;
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
void Server::disable_tcp_blocking() {
	if (inet_protocol == INET_PROTOCOL_TCP) {
		DisableBlocking(server_socket);
	}
}

ServerStartResult Server::start() {
	if (status == SERVER_STATUS_DOWN) {
		status = SERVER_STATUS_STARTING;
		sockaddr_in address;
		FillInaddrStruct(port, address);

		//try to create socket
		server_socket = socket(AF_INET, ConvertProtocol(inet_protocol), 0);
		if (server_socket == INVALID_SOCKET)
			return SERVER_ERROR_ON_INIT;
		//disable blocking
		disable_tcp_blocking();

		if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR){
			CloseSocket(server_socket);
			return SERVER_ERROR_ON_BIND;
		}
		
		if (inet_protocol != INET_PROTOCOL_UDP) {
			if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR){
				CloseSocket(server_socket);
				return SERVER_ERROR_ON_LISTEN;
			}
		}
		//update status
		status = SERVER_STATUS_UP;
		//start server loop
		if (inet_protocol == INET_PROTOCOL_TCP) 
			accept_thread = std::thread([this] {accept_threaded_loop(); });
		data_thread = std::thread([this] {data_threaded_loop(); });
	}
	//return successful
	return SERVER_START_SUCCESSFUL;
}
void Server::stop() {
	if (status == SERVER_STATUS_UP) {
		//close socket forcefully
		CloseSocket(server_socket);
		status = SERVER_STATUS_DOWN;
		for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
			auto& client_pair = *it;
			//disconnect client
			CloseSocket(client_pair.second->GetSocket());
			//free class object
			delete client_pair.second;
		}
		//clear map
		clients.clear();
	}
}
void Server::disconnect(unsigned int id) {
	client_mutex.lock();

	auto it = clients.find(id);
	if (it != clients.end()) {
		ConnectedClient* client = clients.at(id);
		client_disconnect_handler(client, id);
		CloseSocket(client->GetSocket());
		delete clients.at(id);
		clients.erase(id);
	}

	client_mutex.unlock();
}
void Server::sendAll(const char* data, unsigned int size) {
	client_mutex.lock();

	for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
		auto& client_pair = *it;
		sendClientNoLock(client_pair.first, data, size);
	}

	client_mutex.unlock();
}

void Server::sendClient(unsigned int client_id, const char* data, unsigned int size) {
	client_mutex.lock();
	
	sendClientNoLock(client_id, data, size);

	client_mutex.unlock();
}

int Server::sendClientNoLock(unsigned int client_id, const char* data, unsigned int size) {
	auto it = clients.find(client_id);
	int result = 0;
	if (it != clients.end()) {
		if(inet_protocol == INET_PROTOCOL_TCP)
			result = send(clients.at(client_id)->GetSocket(), data, size, 0);
	}

	return result;
}

void Server::accept_threaded_loop() {
	while (status == SERVER_STATUS_UP) {
		sockaddr_in client_address;

		client_mutex.lock();
		unsigned int connected_count = clients.size();
		client_mutex.unlock();
		//accept new client
		SOCKET client_socket = AcceptSocket(server_socket, client_address);
		//check socket
		if (client_socket != INVALID_SOCKET) {
			//check connected clients count
			if (max_connections > 0 && connected_count == max_connections) {
				CloseSocket(client_socket);
				break;
			}
			//new client connected to server
			ConnectedClient* client = new ConnectedClient(client_socket, 
				IPAddress4(GetAddressInteger(client_address)));

			client_mutex.lock();
			unsigned int id = get_random_value(0, 100000);
			while(clients.find(id) != clients.end())
				get_random_value(0, 100000);
			clients.insert(std::make_pair(id, client));
			//call function
			if (client_connect_handler != nullptr)
				client_connect_handler(client, id);	
			client_mutex.unlock();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

void Server::data_threaded_loop() {
	std::vector<unsigned int> ids_to_remove;
	while (status == SERVER_STATUS_UP) {
		client_mutex.lock();
		for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
			auto& client_pair = *it;
			int size = recv(client_pair.second->GetSocket(), buffer, DEFAULT_BUFLEN, 0);
			if (size == 0) {
				//client disconnected
				//call disconnect handler
				if(client_disconnect_handler != nullptr)
					client_disconnect_handler(client_pair.second, client_pair.first);
				//add client's id to delete list
				ids_to_remove.push_back(client_pair.first);
				//break;
			}
			if (size > 0) {
				//some data received from client
				if(client_receive_handler)
					client_receive_handler(client_pair.second, client_pair.first, buffer, size);
			}
			if (size < 0) {
				int error = GetLastSockErrCode();
				if (error == WSAECONNRESET) {
					//client disconnected forcibly
					//call disconnect handler
					if (client_disconnect_handler != nullptr)
						client_disconnect_handler(client_pair.second, client_pair.first);
					//add client's id to delete list
					ids_to_remove.push_back(client_pair.first);
				}
			}
		}

		for (const auto& to_remove : ids_to_remove) {
			auto it = clients.find(to_remove);
			//check iterator
			if (it != clients.end()) {
				auto& client_pair = *it;
				//close socket
				CloseSocket(client_pair.second->GetSocket());
				//free class object
				delete client_pair.second;
				//remove from map
				clients.erase(it);
			}
		}
		ids_to_remove.clear();

		client_mutex.unlock();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}