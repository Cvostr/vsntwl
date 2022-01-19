#include <ConnectedClient.hpp>

using namespace vsntwl;

SOCKET ConnectedClient::getSocket() const {
	return client_socket;
}

const IPAddress4& ConnectedClient::getIP() {
	return client_ip;
}

unsigned short ConnectedClient::getPort() const {
	return client_port;
}

void ConnectedClient::updateConnCheckTime() {
	last_time_conn_check = get_current_time_ms();
}

unsigned int ConnectedClient::getLastConnCheckTime() {
	return last_time_conn_check;
}

ConnectedClient::ConnectedClient(SOCKET sock, IPAddress4 ip, unsigned short port) {
	client_socket = sock;
	client_ip = ip;
	client_port = port;

	updateConnCheckTime();
}