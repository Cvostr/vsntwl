#include <Client.hpp>
#include <iostream>

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
		//connecting socket
		if (SOCKET_ERROR == (connect(client_socket, (sockaddr*)&as_addr, sizeof(as_addr))))
		{
			return CLIENT_CONNECTION_FAILED;
		}

		u_long iMode = 1;
		ioctlsocket(client_socket, FIONBIO, &iMode);

		client_thread = std::thread([this] {client_threaded_loop(); });

		status = CLIENT_STATUS_CONNECTED;
	}

	return CLIENT_CONNECTED;
}
void Client::disconnect() {
	if (status == CLIENT_STATUS_CONNECTED) {
		closesocket(client_socket);
		status = CLIENT_STATUS_DISCONNECTED;
		client_thread.join();
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
		int size = recv(client_socket, buffer, DEFAULT_BUFLEN, 0);
		if (size == 0) {
			//disconnected
			std::cout << "server disconnected me";
			status = CLIENT_STATUS_DISCONNECTED;
		}
		else if (size > 0) {
			//data received
			receive_handler(buffer, size);
		}

	}
}