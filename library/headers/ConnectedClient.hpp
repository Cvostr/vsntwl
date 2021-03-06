#pragma once

#define INCLUDE_SYS_SOCKETS
#include "base.hpp"
#include <IPAddress.hpp>

namespace vsntwl {
	class ConnectedClient {
	private:
		SOCKET client_socket;
		IPAddress4 client_ip;
		unsigned short client_port;
		unsigned int last_time_conn_check;
	public:

		SOCKET getSocket() const;

		const IPAddress4& getIP();

		unsigned short getPort() const;

		void updateConnCheckTime();

		unsigned int getLastConnCheckTime();

		ConnectedClient(SOCKET sock, IPAddress4 ip, unsigned short port);
	};
}