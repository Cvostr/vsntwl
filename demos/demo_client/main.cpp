#include <iostream>
#include <vsntwl.hpp>
#include <TCPClient.hpp>
#include <UDPClient.hpp>
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

	UDPClient* cl = new UDPClient;

	cl->setDataReceivedHandler([](char* data, unsigned int size) {
		std::cout << "Received " << size << " bytes from server " << std::endl;
		for (unsigned int i = 0; i < size; i++) {
			std::cout << data[i];
		}
		std::cout << std::endl;
	});

	cl->setDisconnectHandler([]() {
		std::cout << "Client disconnected from server" << std::endl;
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
			std::cout << "Client disconnected" << std::endl;
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
			break;
		}
	}

	delete cl;

	return 0;
}