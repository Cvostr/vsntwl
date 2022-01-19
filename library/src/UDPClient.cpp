#include <UDPClient.hpp>
#include <iostream>
#include <chrono>
#include <Socket.hpp>

using namespace vsntwl;

UDPClient::UDPClient() : Client() {
	client_thread = nullptr;
	//buffer = new char[DEFAULT_BUFLEN];
}
UDPClient::~UDPClient() {
	if (client_thread) {
		client_thread->join();
		delete client_thread;
	}
}

InetProtocol UDPClient::getInetProtocol() const {
	return INET_PROTOCOL_UDP;
}

ClientConnectResult UDPClient::Connect(IPAddress4 address, unsigned short port) {
	if (status == CLIENT_STATUS_DISCONNECTED) {
		//Creating socket
		client_socket = socket(AF_INET, ConvertProtocol(INET_PROTOCOL_UDP), 0);
		if (INVALID_SOCKET == client_socket)
		{
			return CLIENT_ERROR_SOCKET;
		}
		//Filling struct
		FillInaddrStruct(address, port, server_addr);
		
		//disable blocking
		disable_blocking();

		if (client_thread) {
			client_thread->join();
			delete client_thread;
		}
		//set client status to connected
		status = CLIENT_STATUS_CONNECTED;
		//Send packet to server
		unsigned char welcome = UDP_PREFIX_USER_CONNECTION;
		sendPacket((const char*)&welcome, 1);

		//start client thread
		client_thread = new std::thread([this] {client_threaded_loop(); });
	}

	return CLIENT_CONNECTED;
}

void UDPClient::disconnect() {
	if (status == CLIENT_STATUS_CONNECTED) {
		status = CLIENT_STATUS_DISCONNECTED;
		shutdown(client_socket, 0);
		CloseSocket(client_socket);
		client_socket = INVALID_SOCKET;
	}
}

bool UDPClient::sendData(const char* data, unsigned int size) {
	int	result = SendTo(client_socket, data, size, server_addr);

	packet_stats.sent_packets++;

	if (SOCKET_ERROR == result)
		return false;
	return true;
}

int UDPClient::sendPacket(const char* data, unsigned int size) {
	int result = SendTo(client_socket, data, size, server_addr);

	packet_stats.sent_packets++;
	return result;
}

unsigned int UDPClient::sendReliablePacket(const char* data, unsigned int size) {
	UDP_ReliablePacket* packet = new UDP_ReliablePacket;
	packet->pk_num = packet_stats.sent_packets;
	packet->pk_data = new char[size];
	packet->pk_size = size;
	packet->last_try_time = std::chrono::system_clock::now().time_since_epoch().count();
	memcpy(packet->pk_data, data, size);
	//push packet for waiting
	reliable_packets.push_back(packet);
	//send packet
	sendPacket(data, size);
}

void UDPClient::client_threaded_loop() {
	while (status == CLIENT_STATUS_CONNECTED) {
		client_udp_function();
		//sleep some time
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
}

void UDPClient::client_udp_function() {
	sockaddr_in from;
	int size = RecvFrom(client_socket, buffer, DEFAULT_BUFLEN, from);
	if (size > 0) {
		if (buffer[0] == UDP_PREFIX_ACK) {
			//received ACK packet from server
			//reply ACK

		}

		if (receive_handler != nullptr)
			receive_handler(buffer, size);
	}
}