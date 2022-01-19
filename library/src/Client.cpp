#include <Client.hpp>
#include <iostream>
#include <chrono>
#include <Socket.hpp>

using namespace vsntwl;

Client::Client() {
	client_socket = 0;
	port = 34759;
	status = CLIENT_STATUS_DISCONNECTED;
	buffer = new char[DEFAULT_BUFLEN];
}
Client::~Client() {
	disconnect();

	delete[] buffer;
}

void Client::disable_blocking() {
	DisableBlocking(client_socket);
}

ClientStatus Client::getStatus() const {
	return status;
}

const PacketStats& Client::GetPacketStats() const {
	return packet_stats;
}

void Client::setDataReceivedHandler(client_receive_function const& handler) {
	receive_handler = handler;
}

void Client::setDisconnectHandler(client_disconnect_function const& handler) {
	disconnect_handler = handler;
}