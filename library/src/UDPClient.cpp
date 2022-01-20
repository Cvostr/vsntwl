#include <UDPClient.hpp>
#include <iostream>
#include <chrono>
#include <Socket.hpp>
#include <string.h>

using namespace vsntwl;

UDPClient::UDPClient() : Client() {
	client_thread = nullptr;
	last_time_conn_check = 0;
	temp_send_buffer = new char[DEFAULT_BUFLEN];
}
UDPClient::~UDPClient() {
	delete[] temp_send_buffer;
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
		status = CLIENT_STATUS_CONNECTING;
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
		bool result = waitForConnection();
		if (result) {
			//set client status to connected
			status = CLIENT_STATUS_CONNECTED;

			last_time_conn_check = get_current_time_ms();
			//start client thread
			client_thread = new std::thread([this] {client_threaded_loop(); });
		}
		else {
			status = CLIENT_STATUS_DISCONNECTED;
			return CLIENT_ERROR_CONNECTION;
		}
	}

	return CLIENT_CONNECTED;
}

bool UDPClient::waitForConnection() {
	//Send packet to server
	unsigned char welcome = UDP_PREFIX_USER_CONNECTION;
	sendPacket((const char*)&welcome, 1);

	return waitForConnAck();
}

bool UDPClient::waitForDisconnection() {
	//Send packet to server
	unsigned char bye = UDP_PREFIX_USER_DISCONNECTION;
	sendPacket((const char*)&bye, 1);

	return true;
}

bool UDPClient::waitForConnAck(char prefix, unsigned int timeout) {
	unsigned int start_time = get_current_time_ms();
	unsigned int current_time = start_time;
	while ((current_time - start_time) < timeout) {
		char answer = 0;
		receivePacket(&answer, 1);
		if (answer == prefix)
			return true;
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		current_time = get_current_time_ms();
	}
	return false;
}

void UDPClient::disconnect() {
	if (status == CLIENT_STATUS_CONNECTED) {
		bool result = waitForDisconnection();
		if (result) {
			status = CLIENT_STATUS_DISCONNECTED;
			shutdown(client_socket, 0);
			CloseSocket(client_socket);
			client_socket = INVALID_SOCKET;
		}
	}
}

bool UDPClient::sendData(const char* data, unsigned int size) {
	temp_send_buffer[0] = UDP_PREFIX_PACKET;
	*((int*)(&temp_send_buffer[1])) = packet_stats.sent_packets;
	memcpy(&temp_send_buffer[5], data, size);

	int	result = SendTo(client_socket, temp_send_buffer, size + 5, server_addr);

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

int UDPClient::receivePacket(char* data, unsigned int size) {
	sockaddr_in from;
	int result = RecvFrom(client_socket, data, size, from);
	return result;
}

unsigned int UDPClient::sendReliablePacket(const char* data, unsigned int size) {
	UDP_ReliablePacket* packet = new UDP_ReliablePacket;
	packet->pk_num = packet_stats.sent_packets;
	packet->pk_data = new char[size];
	packet->pk_size = size;
	packet->last_try_time = get_current_time_ms();
	memcpy(packet->pk_data, data, size);
	//push packet for waiting
	reliable_packets.push_back(packet);
	//send packet
	sendPacket(data, size);

	return 0;
}

void UDPClient::client_threaded_loop() {
	while (status == CLIENT_STATUS_CONNECTED) {
		client_udp_function();
		//sleep some time
		sleep_cur_thread_ms(2);
	}
}

void UDPClient::client_udp_function() {
	if ((get_current_time_ms() - last_time_conn_check) > UDP_CONN_CHECK_TIMEOUT) {
		if (disconnect_handler != nullptr)
			disconnect_handler();
		disconnect();
	}
	sockaddr_in from;
	int size = RecvFrom(client_socket, buffer, DEFAULT_BUFLEN, from);
	if (size > 0) {
		if (buffer[0] == UDP_PREFIX_CONN_CHECK) {
			//received Check packet from server
			last_time_conn_check = get_current_time_ms();
			//reply answer with same prefix
			sendPacket(buffer, 1);
		}
		else if (buffer[0] == UDP_PREFIX_PACKET) {
			if (receive_handler != nullptr)
				receive_handler(buffer + 5, size - 5);
		}
	}
}