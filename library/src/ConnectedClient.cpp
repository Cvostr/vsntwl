#include <ConnectedClient.hpp>

using namespace vsntwl;

SOCKET ConnectedClient::getSocket() const {
	return client_socket;
}

const IPAddress4& ConnectedClient::getIP() {
	return client_ip;
}

unsigned short ConnectedClient::getPort() {
	return client_port;
}

ConnectedClient::ConnectedClient(SOCKET sock, IPAddress4 ip, unsigned short port) {
	client_socket = sock;
	client_ip = ip;
	client_port = port;
}