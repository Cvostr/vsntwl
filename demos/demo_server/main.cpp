#include <iostream>
#include <conio.h>
#include <vsntwl.hpp>
#include <Server.hpp>

using namespace vsntwl;

int main(int argc, char** argv) {
	init();

	Server* serv = new Server;

	serv->setClientConnectedHandler([](ConnectedClient* client, unsigned int id) {
		std::cout << "Connected client " << id << std::endl;
	});

	serv->setClientDataReceiveHandler([](ConnectedClient* client, unsigned int id, char* data, unsigned int size) {
		std::cout << "Received " << size << " bytes from client " << id << std::endl;
		for (unsigned int i = 0; i < size; i++) {
			std::cout << data[i];
		}
		std::cout << std::endl;
	});

	serv->setClientDisconnectedHandler([](ConnectedClient* client, unsigned int id) {
		std::cout << "Disconnected client " << id << std::endl;
	});

	ServerStartResult result = serv->start();
	if (result > 0) {
		std::cout << "Error server opening " << result << std::endl;
	}else
		std::cout << "Server started successfully" << std::endl;

	std::cout << "Press S to send 4 bytes to all clients" << std::endl;

	int p = 43212;
	while (true) {
		char ch = getch();
		if (ch == 's') {
			serv->sendAll((const char*)&p, 4);
			std::cout << "Sent 4 bytes to all clients" << std::endl;
		}
		if (ch == 'k') {
			std::cout << "Type id of client" << std::endl;
			int id;
			std::cin >> id;
			serv->disconnect(id);
		}
	}

	serv->stop();

	return 0;
}