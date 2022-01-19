#include <UDPServer.hpp>
#include <Socket.hpp>

using namespace vsntwl;

UDPServer::UDPServer() : Server() {

}

UDPServer::~UDPServer() {
	stop();
}

InetProtocol UDPServer::getInetProtocol() const {
	return INET_PROTOCOL_UDP;
}

ServerStartResult UDPServer::start() {
	if (status == SERVER_STATUS_DOWN) {
		status = SERVER_STATUS_STARTING;
		sockaddr_in address;
		FillInaddrStruct(port, address);

		//try to create socket
		server_socket = socket(AF_INET, ConvertProtocol(INET_PROTOCOL_UDP), 0);
		if (server_socket == INVALID_SOCKET)
			return SERVER_ERROR_ON_INIT;
		//disable blocking
		disable_socket_blocking();

		if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
			CloseSocket(server_socket);
			return SERVER_ERROR_ON_BIND;
		}

		//update status
		status = SERVER_STATUS_UP;
		//start server loop
		data_thread = std::thread([this] {data_threaded_loop(); });
	}
	//return successful
	return SERVER_START_SUCCESSFUL;
}

void UDPServer::stop() {
	if (status == SERVER_STATUS_UP) {
		//close socket forcefully
		CloseSocket(server_socket);
		status = SERVER_STATUS_DOWN;
		client_mutex.lock();
		for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
			auto& client_pair = *it;
			//disconnect client
			SOCKET sock = client_pair.second->getSocket();
			if (sock != INVALID_SOCKET)
				CloseSocket(sock);
			//free class object
			delete client_pair.second;
		}
		//clear map
		clients.clear();
		client_mutex.unlock();
	}
}

void UDPServer::disconnect(unsigned int id) {
	client_mutex.lock();

	auto it = clients.find(id);
	if (it != clients.end()) {
		ConnectedClient* client = clients.at(id);
		client_disconnect_handler(client, id);

		SOCKET socket = client->getSocket();
		if (socket != INVALID_SOCKET)
			CloseSocket(socket);

		delete clients.at(id);
		clients.erase(id);
	}

	client_mutex.unlock();
}

void UDPServer::sendAll(const char* data, unsigned int size) {
	client_mutex.lock();

	for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
		auto& client_pair = *it;
		sendClientNoLock(client_pair.first, data, size);
	}

	client_mutex.unlock();
}

void UDPServer::sendClient(unsigned int client_id, const char* data, unsigned int size) {
	client_mutex.lock();

	sendClientNoLock(client_id, data, size);

	client_mutex.unlock();
}

int UDPServer::sendClientNoLock(unsigned int client_id, const char* data, unsigned int size) {
	auto it = clients.find(client_id);
	int result = 0;
	if (it != clients.end()) {
		auto& pair = *it;
		sockaddr_in dest;
		FillInaddrStruct(pair.second->getIP(), pair.second->getPort(), dest);
		dest.sin_port = pair.second->getPort();
		SendTo(server_socket, data, size, dest);
	}

	return result;
}

void UDPServer::data_threaded_loop() {
	while (status == SERVER_STATUS_UP) {
		//Receive UDP package
		sockaddr_in sender;
		int result = RecvFrom(server_socket, buffer, DEFAULT_BUFLEN, sender);
		
		if (result >= 0) {
			client_mutex.lock();
			auto client = getConnectionByAddress(
				IPAddress4(GetAddressInteger(sender)), sender.sin_port);
			client_mutex.unlock();

			if (client.second == nullptr) {
				ConnectedClient* client = new ConnectedClient(INVALID_SOCKET,
					IPAddress4(GetAddressInteger(sender)), sender.sin_port);

				if (buffer[0] == UDP_PREFIX_USER_CONNECTION) {

					client_mutex.lock();
					unsigned int id = get_random_value(0, 100000);
					while (clients.find(id) != clients.end())
						get_random_value(0, 100000);
					clients.insert(std::make_pair(id, client));
					//call function
					if (client_connect_handler != nullptr)
						client_connect_handler(client, id);
					client_mutex.unlock();
				}
			}
			else {
				client_mutex.lock();
				if (client_receive_handler)
					client_receive_handler(client.second, client.first, buffer, result);
				client_mutex.unlock();
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}
