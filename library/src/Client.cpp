#include <Client.hpp>
#include <iostream>
#include <chrono>

using namespace vsntwl;

Client::Client() {
	client_socket = 0;
	port = 34759;
	inet_protocol = INET_PROTOCOL_TCP;
	status = CLIENT_STATUS_DISCONNECTED;

	buffer = new char[DEFAULT_BUFLEN];
}
Client::~Client() {
	delete[] buffer;
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
	u_long iMode = 1;
	ioctlsocket(client_socket, FIONBIO, &iMode);
}
ClientConnectResult Client::Connect(IPAddress4 address, unsigned short port) {
	if (status == CLIENT_STATUS_DISCONNECTED) {
		//Creating socket
		client_socket = socket(AF_INET, ConvertProtocol(inet_protocol), 0);
		if (INVALID_SOCKET == client_socket)
		{
			return CLIENT_CONNECTION_FAILED;
		}
		//Filling struct
		sockaddr_in as_addr;
		ZeroMemory(&as_addr, sizeof(as_addr));
		as_addr.sin_family = AF_INET;
		as_addr.sin_addr.S_un.S_addr = address.ip;
		as_addr.sin_port = htons(port);
		//connecting to server, no need to do this on UDP
		if (inet_protocol == INET_PROTOCOL_TCP) {
			//connecting socket
			if (SOCKET_ERROR == (connect(client_socket, (sockaddr*)&as_addr, sizeof(as_addr))))
			{
				return CLIENT_CONNECTION_FAILED;
			}
			//disable blocking
			disable_tcp_blocking();
		}
		//set client status to connected
		status = CLIENT_STATUS_CONNECTED;
		//start client thread
		if (client_thread.joinable())
			client_thread.join();
		client_thread = std::thread([this] {client_threaded_loop(); });
	}

	return CLIENT_CONNECTED;
}
void Client::disconnect() {
	if (status == CLIENT_STATUS_CONNECTED) {
		closesocket(client_socket);
		status = CLIENT_STATUS_DISCONNECTED;
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
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void Client::client_tcp_function() {
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
		int error = WSAGetLastError();
		if (error == WSAECONNRESET) {
			//disconnected from server (server closed forcibly)
			if (disconnect_handler != nullptr)
				disconnect_handler();
			disconnect();
		}
	}
}

void Client::client_udp_function() {
	sockaddr from;
	int from_len = 0;
	int size = recvfrom(client_socket, buffer, DEFAULT_BUFLEN, 0, &from, &from_len);
	if (size > 0) {
		if (receive_handler != nullptr)
			receive_handler(buffer, size);
	}
}