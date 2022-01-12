#include <iostream>
#include <vsntwl.hpp>
#include <Server.hpp>
#include <Socket.hpp>
#include <stdio.h>

#ifdef _WIN32
#include <conio.h>
#endif

#ifdef __linux__
#include <termios.h>
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
#endif

using namespace vsntwl;

int main(int argc, char** argv) {
	init();

	Server* serv = new Server;
	serv->setInetProtocol(INET_PROTOCOL_UDP);
	//serv->setMaxConnections(2);
	serv->setClientConnectedHandler([](ConnectedClient* client, unsigned int id) {
		std::cout << "Connected client " << id << std::endl;
	});

	serv->setClientDataReceiveHandler([serv](ConnectedClient* client, unsigned int id, char* data, unsigned int size) {
		std::cout << "Received " << size << " bytes from client " << id << std::endl;
		for (unsigned int i = 0; i < size; i++) {
			std::cout << data[i];
		}
		std::cout << std::endl;

		char* answ = "answer to client";

		//serv->sendClientNoLock(id, answ, 16);
	});

	serv->setClientDisconnectedHandler([](ConnectedClient* client, unsigned int id) {
		std::cout << "Disconnected client " << id << std::endl;
	});

	ServerStartResult result = serv->start();
	if (result > 0) {
		std::cout << "Error server opening " << result << " " << GetLastSockErrCode() << std::endl;
	}else
		std::cout << "Server started successfully" << std::endl;

	std::cout << "Press S to send 4 bytes to all clients" << std::endl;
	std::cout << "Press K to disconnect client by its id" << std::endl;
	std::cout << "Press D to stop server" << std::endl;

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
		if (ch == 'd') {
			serv->stop();
			std::cout << "Server stopped" << std::endl;
		}
	}

	serv->stop();

	return 0;
}