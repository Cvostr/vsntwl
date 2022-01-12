#include <Client.hpp>
#include <iostream>
#include <chrono>
#include <Socket.hpp>

using namespace vsntwl;

Client::Client() {
	client_socket = 0;
	port = 34759;
	inet_protocol = INET_PROTOCOL_TCP;
	status = CLIENT_STATUS_DISCONNECTED;
	client_thread = nullptr;
	buffer = new char[DEFAULT_BUFLEN];
}
Client::~Client() {
	disconnect();

	delete[] buffer;

	if (client_thread) {
		client_thread->join();
		delete client_thread;
	}
}

void Client::setInetProtocol(InetProtocol protocol) {
	inet_protocol = protocol;
}

InetProtocol Client::getInetProtocol() const {
	return inet_protocol;
}

ClientStatus Client::getStatus() const {
	return status;
}

void Client::setDataReceivedHandler(client_receive_function const& handler) {
	receive_handler = handler;
}

void Client::setDisconnectHandler(client_disconnect_function const& handler) {
	disconnect_handler = handler;
}
void Client::disable_tcp_blocking() {
	DisableBlocking(client_socket);
}
ClientConnectResult Client::Connect(IPAddress4 address, unsigned short port) {
	if (status == CLIENT_STATUS_DISCONNECTED) {
		//Creating socket
		client_socket = socket(AF_INET, ConvertProtocol(inet_protocol), 0);
		if (INVALID_SOCKET == client_socket)
		{
			return CLIENT_ERROR_SOCKET;
		}
		//Filling struct
		sockaddr_in as_addr;
		FillInaddrStruct(address, port, as_addr);
		//connecting to server, no need to do this on UDP
		if (inet_protocol == INET_PROTOCOL_TCP) {
			//connecting socket
			if (SOCKET_ERROR == (connect(client_socket, (sockaddr*)&as_addr, sizeof(as_addr))))
			{
				CloseSocket(client_socket);
				return CLIENT_ERROR_CONNECTION;
			}
			//disable blocking
			disable_tcp_blocking();
		}

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
void Client::disconnect() {
	if (status == CLIENT_STATUS_CONNECTED) {
		status = CLIENT_STATUS_DISCONNECTED;
		shutdown(client_socket, 0);
		CloseSocket(client_socket);
		client_socket = INVALID_SOCKET;
	}
}

bool Client::sendData(const char* data, unsigned int size) {
	int result = send(client_socket, data, size, 0);
	if (SOCKET_ERROR == result)
		return false;
	return true;
}

void Client::client_threaded_loop() {
	while (status == CLIENT_STATUS_CONNECTED) {
		if (inet_protocol == INET_PROTOCOL_TCP)
			client_tcp_function();
		else if (inet_protocol == INET_PROTOCOL_UDP)
			client_udp_function();
		//sleep some time
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
	}
}

void Client::client_tcp_function() {
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

void Client::client_udp_function() {
	sockaddr_in from;
	int from_len = 0;
	int size = RecvFrom(client_socket, buffer, DEFAULT_BUFLEN, from);
	if (size > 0) {
		if (receive_handler != nullptr)
			receive_handler(buffer, size);
	}
}