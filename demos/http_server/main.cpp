#include <iostream>
#include <vsntwl.hpp>
#include <TCPServer.hpp>
#include <Socket.hpp>
#include <stdio.h>
#include "http_message.hpp"
#include <fstream>

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

std::string sample_page = std::string("<!DOCTYPE>") + "<html>" +
"  <head>" +
"    <title>VSNTWL Http server example</title>" +
"  </head>" +
"  <body>" +
" <body bgcolor=\"green\""+
"    <h3>Hello from vsntwl</h3>" +
"    <form method=\"post\" action=\"button\">" +
"      <input type=\"submit\" value=\"Click Me!\">" +
"    </form>" +
"  </body>" +
"</html>";

int main(int argc, char** argv) {
	init();

	TCPServer* serv = new TCPServer;
	serv->setPort(80); //Common HTTP port
	serv->setClientConnectedHandler([](ConnectedClient* client, unsigned int id) {
		std::cout << "Connected client " << id << std::endl;
	});

	serv->setClientDataReceiveHandler([serv](ConnectedClient* client, unsigned int id, char* data, unsigned int size) {
		std::cout << "Received " << size << " bytes from client " << id << std::endl;
		std::string request_str;
		for (unsigned int i = 0; i < size; i++) {
			request_str.push_back(data[i]);
		}
		HttpRequest request(request_str);
		if (request.getUrl() == "/") {
			HttpResponse response(request.getVersion(), 200, sample_page);
			response.Create();
			serv->sendClientNoLock(id, response.getContent().c_str(), response.getContent().size());
		}
		else {
			std::ifstream stream(request.getUrl().substr(1), std::ios_base::binary | std::ios_base::ate);
			if (stream.fail()) {
				HttpResponse response(request.getVersion(), 404, "");
				response.Create();
				serv->sendClientNoLock(id, response.getContent().c_str(), response.getContent().size());
				return;
			}
			size_t size = stream.tellg();
			stream.seekg(0, std::ios_base::beg);
			char* data = new char[size];
			stream.read(data, size);
			std::string data_str;
			for (unsigned int i = 0; i < size; i++) {
				data_str.push_back(data[i]);
			}
			HttpResponse response(request.getVersion(), 200, data_str);
			response.Create();
			serv->sendClientNoLock(id, response.getContent().c_str(), response.getContent().size());

		}
	});

	serv->setClientDisconnectedHandler([](ConnectedClient* client, unsigned int id) {
		std::cout << "Disconnected client " << id << std::endl;
	});

	ServerStartResult result = serv->start();
	if (result > 0) {
		std::cout << "Error server opening " << result << " " << GetLastSockErrCode() << std::endl;
	}else
		std::cout << "Server started successfully" << std::endl;

	std::cout << "Press D to stop server" << std::endl;

	while (true) {
		char ch = getch();
		if (ch == 'd') {
			serv->stop();
			std::cout << "Server stopped" << std::endl;
		}
	}

	return 0;
}