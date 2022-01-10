#include <iostream>
#include <conio.h>
#include <vsntwl.hpp>
#include <Client.hpp>

using namespace vsntwl;

void connect(Client* client, IPAddress4 addr) {
	ClientConnectResult result = client->Connect(addr, 34759);

	if (result != 0) {
		std::cout << "Client not connected " << std::endl;
	}
	else
		std::cout << "Client connected to server " << std::endl;
}

int main(int argc, char** argv) {

	init();

	Client* cl = new Client;

	//cl->setInetProtocol(INET_PROTOCOL_UDP);

	cl->setDataReceivedHandler([](char* data, unsigned int size) {
		std::cout << "Received " << size << " bytes from server " << std::endl;
		for (unsigned int i = 0; i < size; i++) {
			std::cout << data[i];
		}
		std::cout << std::endl;
	});

	cl->setDisconnectHandler([]() {
		std::cout << "Server disconnected client" << std::endl;
	});

	connect(cl, IPAddress4());

	std::cout << "Press C to connect" << std::endl;
	std::cout << "Press S to send 4 bytes to server" << std::endl;
	std::cout << "Press T to send something to server" << std::endl;
	std::cout << "Press D to disconnect" << std::endl;
	std::cout << "Press E to exit" << std::endl;

	int p = 23512;

	while (true) {
		char ch = getch();
		if (ch == 's') {
			cl->sendData((const char*)&p, 4);
			std::cout << "Sent 4 bytes to server" << std::endl;
		}
		if (ch == 't') {
			std::cout << "Type text to send" << std::endl;
			std::string text;
			std::getline(std::cin, text);
			cl->sendData(text.c_str(), text.size());
			std::cout << "Sent 4 bytes to server" << std::endl;
		}
		if (ch == 'd') {
			cl->disconnect();
			std::cout << "Disconnected from server" << std::endl;
		}
		if (ch == 'c')
			connect(cl, IPAddress4());
		if (ch == 'p') {
			std::cout << "Type IP address" << std::endl;
			std::string ip;
			std::cin >> ip;
			IPAddress4 addr(ip);
			connect(cl, addr);
		}
		if (ch == 'e') {
			cl->disconnect();
			return 0;
		}
	}

	return 0;
}