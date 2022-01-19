#include <TCPClient.hpp>
#include <iostream>
#include <chrono>
#include <Socket.hpp>

using namespace vsntwl;

TCPClient::TCPClient() : Client() {
	client_thread = nullptr;
	buffer = new char[DEFAULT_BUFLEN];
}
TCPClient::~TCPClient() {
	if (client_thread) {
		client_thread->join();
		delete client_thread;
	}
}

InetProtocol TCPClient::getInetProtocol() const {
	return INET_PROTOCOL_TCP;
}

ClientConnectResult TCPClient::Connect(IPAddress4 address, unsigned short port) {
	if (status == CLIENT_STATUS_DISCONNECTED) {
		//Creating socket
		client_socket = socket(AF_INET, ConvertProtocol(INET_PROTOCOL_TCP), 0);
		if (INVALID_SOCKET == client_socket)
		{
			return CLIENT_ERROR_SOCKET;
		}
		//Filling struct
		sockaddr_in as_addr;
		FillInaddrStruct(address, port, as_addr);
		//connecting to server
		if (SOCKET_ERROR == (connect(client_socket, (sockaddr*)&as_addr, sizeof(as_addr))))
		{
			CloseSocket(client_socket);
			return CLIENT_ERROR_CONNECTION;
		}
		//disable blocking
		disable_blocking();

		if (client_thread) {
			client_thread->join();
			delete client_thread;
		}
		//set client status to connected
		status = CLIENT_STATUS_CONNECTED;
		//start client thread
		client_thread = new std::thread([this] {client_threaded_loop(); });
	}

	return CLIENT_CONNECTED;
}
void TCPClient::disconnect() {
	if (status == CLIENT_STATUS_CONNECTED) {
		status = CLIENT_STATUS_DISCONNECTED;
		shutdown(client_socket, 0);
		CloseSocket(client_socket);
		client_socket = INVALID_SOCKET;
	}
}

bool TCPClient::sendData(const char* data, unsigned int size) {
	int result = 0;

	result = send(client_socket, data, size, 0);

	packet_stats.sent_packets++;

	if (SOCKET_ERROR == result)
		return false;
	return true;
}

int TCPClient::sendPacket(const char* data, unsigned int size) {
	int result = send(client_socket, data, size, 0);

	packet_stats.sent_packets++;
	return result;
}

void TCPClient::client_threaded_loop() {
	while (status == CLIENT_STATUS_CONNECTED) {
		client_tcp_function();
		//sleep some time
		std::this_thread::sleep_for(std::chrono::milliseconds(3));
	}
}

void TCPClient::client_tcp_function() {
	if (client_socket == INVALID_SOCKET)
		return;
	int size = recv(client_socket, buffer, DEFAULT_BUFLEN, 0);
	if (size == 0) {
		//disconnected by server manually
		if (disconnect_handler != nullptr)
			disconnect_handler();
		disconnect();
	}
	else if (size > 0) {
		packet_stats.received_packets++;
		//data received
		if (receive_handler != nullptr)
			receive_handler(buffer, size);
	}
	else if (size < 0) {
		int error = GetLastSockErrCode();
		if (error == WSAECONNRESET) {
			//disconnected from server (server closed forcibly)
			if (disconnect_handler != nullptr)
				disconnect_handler();
			disconnect();
		}
	}
}