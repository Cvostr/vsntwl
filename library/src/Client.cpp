#include <Client.hpp>

using namespace vsntwl;

Client::Client() {
	client_socket = 0;
	port = 34759;
	inet_protocol = INET_PROTOCOL_TCP;
}
Client::~Client() {

}

void Client::setInetProtocol(InetProtocol protocol) {
	inet_protocol = protocol;
}
InetProtocol Client::getInetProtocol() const {
	return inet_protocol;
}

ClientConnectResult Client::Connect(IPAddress4 address, unsigned short port) {
	//Creating socket
	client_socket = socket(AF_INET, ConvertProtocol(inet_protocol), 0);
	if (INVALID_SOCKET == client_socket)
	{
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

	return CLIENT_CONNECTED;
}
void Client::disconnect() {
	closesocket(client_socket);
}

bool Client::sendData(const char* data, unsigned int size) {
	int result = send(client_socket, data, size, 0);
	if (SOCKET_ERROR == result)
		return false;
	return true;
}