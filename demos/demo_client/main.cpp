#include <iostream>
#include <conio.h>
#include <vsntwl.hpp>
#include <Client.hpp>

using namespace vsntwl;

void connect(Client* client) {
	ClientConnectResult result = client->Connect(IPAddress4(), 34759);

	if (result != 0) {
		std::cout << "Client not connected " << std::endl;
	}
	else
		std::cout << "Client connected to server " << std::endl;
}

int main(int argc, char** argv) {

	init();

	Client* cl = new Client;
	connect(cl);

	std::cout << "Press C to connect" << std::endl;
	std::cout << "Press S to send 4 bytes to server" << std::endl;
	std::cout << "Press T to send something to server" << std::endl;
	std::cout << "Press D to disconnect" << std::endl;
	std::cout << "Press E to exit" << std::endl;

	int p = 12;

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
		if (ch == 'd')
			cl->disconnect();
		if (ch == 'c')
			connect(cl);
		if (ch == 'e')
			return 0;
	}

	return 0;
}