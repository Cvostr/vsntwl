#include <UDPServer.hpp>
#include <Socket.hpp>

using namespace vsntwl;

UDPServer::UDPServer() : Server() {
	temp_send_buffer = new char[DEFAULT_BUFLEN];
}

UDPServer::~UDPServer() {
	delete[] temp_send_buffer;
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

		clients_check_thread = std::thread([this] {clients_check_loop();  });
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

int UDPServer::sendPacketClientNoLock(unsigned int client_id, const char* data, unsigned int size) {
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

int UDPServer::sendClientNoLock(unsigned int client_id, const char* data, unsigned int size) {
	auto it = clients.find(client_id);
	int result = 0;
	if (it != clients.end()) {
		temp_send_buffer[0] = UDP_PREFIX_PACKET;
		//*((int*)(&temp_send_buffer[1])) = packet_stats.sent_packets;
		memcpy(&temp_send_buffer[5], data, size);

		auto& pair = *it;
		sockaddr_in dest;
		FillInaddrStruct(pair.second->getIP(), pair.second->getPort(), dest);
		dest.sin_port = pair.second->getPort();
		SendTo(server_socket, temp_send_buffer, size + 5, dest);
	}

	return result;
}

void UDPServer::clients_check_loop() {
	std::vector<unsigned int> ids_to_remove;
	while (status == SERVER_STATUS_UP) {
		client_mutex.lock();

		for (auto it = clients.begin(), end = clients.end(); it != end; ++it) {
			auto& client = *it;
			char conn_check = UDP_PREFIX_CONN_CHECK;
			sendPacketClientNoLock(client.first, &conn_check, 1);
			unsigned int diff = get_current_time_ms() - client.second->getLastConnCheckTime();
			if (diff > UDP_CONN_CHECK_TIMEOUT) {
				//call disconnection handler
				if (client_disconnect_handler != nullptr)
					client_disconnect_handler(client.second, client.first);
				//add client's id to delete list
				ids_to_remove.push_back(client.first);
			}
		}
		//remove clients
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
		sleep_cur_thread_ms(60);
	}
}

void UDPServer::data_threaded_loop() {
	std::vector<unsigned int> ids_to_remove;
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
				if (buffer[0] == UDP_PREFIX_USER_CONNECTION) {
					ConnectedClient* client = new ConnectedClient(INVALID_SOCKET,
						IPAddress4(GetAddressInteger(sender)), sender.sin_port);

					client_mutex.lock();
					unsigned int id = get_random_value(0, 100000);
					while (clients.find(id) != clients.end())
						get_random_value(0, 100000);
					clients.insert(std::make_pair(id, client));
					//call function
					if (client_connect_handler != nullptr)
						client_connect_handler(client, id);

					char ack = UDP_PREFIX_CONN_ACK;
					sendPacketClientNoLock(id, &ack, 1);
					client_mutex.unlock();
				}
			}
			else {
				char prefix = buffer[0];
				if (prefix == UDP_PREFIX_USER_DISCONNECTION) {
					char ack = UDP_PREFIX_DISCONN_ACK;
					client_mutex.lock();
					sendPacketClientNoLock(client.first, &ack, 1);

					if (client_disconnect_handler != nullptr)
						client_disconnect_handler(client.second, client.first);
					client_mutex.unlock();
					//add client's id to delete list
					ids_to_remove.push_back(client.first);
				}
				else if (prefix == UDP_PREFIX_CONN_CHECK) {
					client.second->updateConnCheckTime();
				}
				else if (prefix == UDP_PREFIX_PACKET) {
					client_mutex.lock();
					if (client_receive_handler)
						client_receive_handler(client.second, client.first, buffer + 5, result - 5);
					client_mutex.unlock();
				}
			}
		}

		for (const auto& to_remove : ids_to_remove) {
			client_mutex.lock();
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
			client_mutex.unlock();
		}
		ids_to_remove.clear();

		sleep_cur_thread_ms(5);
	}
}
