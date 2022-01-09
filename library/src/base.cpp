#define INCLUDE_SYS_SOCKETS
#include <base.hpp>

int ConvertProtocol(InetProtocol protocol) {
	int result = 0;
#ifdef _WIN32
	switch (protocol) {
	case INET_PROTOCOL_TCP:
		result = SOCK_STREAM;
		break;
	case INET_PROTOCOL_UDP:
		result = SOCK_DGRAM;
		break;
	}
#endif
	return result;
}