#include <TCPServer.hpp>
#include <Socket.hpp>

using namespace vsntwl;

TCPServer::TCPServer() : Server() {

}

TCPServer::~TCPServer() {
	stop();
}

InetProtocol TCPServer::getInetProtocol() const {
	return INET_PROTOCOL_TCP;
}

ServerStartResult TCPServer::start() {
	if (status == SERVER_STATUS_DOWN) {
		status = SERVER_STATUS_STARTING;
		sockaddr_in address;
		FillInaddrStruct(port, address);

		//try to create socket
		server_socket = socket(AF_INET, ConvertProtocol(INET_PROTOCOL_TCP), 0);
		if (server_socket == INVALID_SOCKET)
			return SERVER_ERROR_ON_INIT;
		//disable blocking
		disable_socket_blocking();

		if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR){
			CloseSocket(server_socket);
			return SERVER_ERROR_ON_BIND;
		}
		
		//if (inet_protocol != INET_PROTOCOL_UDP) {
			if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR){
				CloseSocket(server_socket);
				return SERVER_ERROR_ON_LISTEN;
			}
		//}
		//update status
		status = SERVER_STATUS_UP;
		//start server loop
		//if (inet_protocol == INET_PROTOCOL_TCP) 
			accept_thread = std::thread([this] {accept_threaded_loop(); });
		//if (inet_protocol == INET_PROTOCOL_TCP)
			data_thread = std::thread([this] {data_threaded_loop(); });
		//else
		//	data_thread = std::thread([this] {data_threaded_udp_loop(); });
	}
	//return successful
	return SERVER_START_SUCCESSFUL;
}

void TCPServer::stop() {
	if (status == SERVER_STATUS_UP) {
		//close socket forcefully
		CloseSocket(server_socket);
		status = SERVER_STATUS_DOWN;
		client_mutex.lock();
		for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
			auto& client_pair = *it;
			//disconnect client
			SOCKET sock = client_pair.second->getSocket();
			if(sock != INVALID_SOCKET)
				CloseSocket(sock);
			//free class object
			delete client_pair.second;
		}
		//clear map
		clients.clear();
		client_mutex.unlock();
	}
}

void TCPServer::disconnect(unsigned int id) {
	client_mutex.lock();

	auto it = clients.find(id);
	if (it != clients.end()) {
		ConnectedClient* client = clients.at(id);
		client_disconnect_handler(client, id);
		
		SOCKET socket = client->getSocket();
		if(socket != INVALID_SOCKET)
			CloseSocket(socket);

		delete clients.at(id);
		clients.erase(id);
	}

	client_mutex.unlock();
}

void TCPServer::sendAll(const char* data, unsigned int size) {
	client_mutex.lock();

	for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
		auto& client_pair = *it;
		sendClientNoLock(client_pair.first, data, size);
	}

	client_mutex.unlock();
}

void TCPServer::sendClient(unsigned int client_id, const char* data, unsigned int size) {
	client_mutex.lock();

	sendClientNoLock(client_id, data, size);

	client_mutex.unlock();
}

int TCPServer::sendClientNoLock(unsigned int client_id, const char* data, unsigned int size) {
	auto it = clients.find(client_id);
	int result = 0;
	if (it != clients.end()) {
		//if(inet_protocol == INET_PROTOCOL_TCP)
		result = send(clients.at(client_id)->getSocket(), data, size, 0);
		/*else if (inet_protocol == INET_PROTOCOL_UDP) {
			auto& pair = *it;
			sockaddr_in dest;
			FillInaddrStruct(pair.second->getIP(), pair.second->getPort(), dest);
			dest.sin_port = pair.second->getPort();
			SendTo(server_socket, data, size, dest);
		}*/
	}

	return result;
}


void TCPServer::accept_threaded_loop() {
	while (status == SERVER_STATUS_UP) {
		sockaddr_in client_address;
		//get current connected clients
		client_mutex.lock();
		unsigned int connected_count = clients.size();
		client_mutex.unlock();
		//accept new client
		SOCKET client_socket = AcceptSocket(server_socket, client_address);
		//disable blocking on client socket
		DisableBlocking(client_socket);
		//check socket
		if (client_socket != INVALID_SOCKET) {
			//check connected clients count
			//if limit reached, disconnect client
			if (max_connections > 0 && connected_count == max_connections) {
				CloseSocket(client_socket);
			}
			else {
				//new client connected to server
				ConnectedClient* client = new ConnectedClient(client_socket,
					IPAddress4(GetAddressInteger(client_address)), client_address.sin_port);
				//Add new client
				client_mutex.lock();
				//generate random client ID
				unsigned int id = get_random_value(0, 100000);
				while (clients.find(id) != clients.end())
					get_random_value(0, 100000);
				//insert client to map
				clients.insert(std::make_pair(id, client));
				//call function
				if (client_connect_handler != nullptr)
					client_connect_handler(client, id);
				//unlock mutex
				client_mutex.unlock();
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

void TCPServer::data_threaded_loop() {
	std::vector<unsigned int> ids_to_remove;
	while (status == SERVER_STATUS_UP) {
		client_mutex.lock();
		for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
			auto& client_pair = *it;
			int size = recv(client_pair.second->getSocket(), buffer, DEFAULT_BUFLEN, 0);
			if (size == 0) {
				//client disconnected
				//call disconnect handler
				if (client_disconnect_handler != nullptr)
					client_disconnect_handler(client_pair.second, client_pair.first);
				//add client's id to delete list
				ids_to_remove.push_back(client_pair.first);
				//break;
			}
			if (size > 0) {
				//some data received from client
				if (client_receive_handler)
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
				CloseSocket(client_pair.second->getSocket());
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
