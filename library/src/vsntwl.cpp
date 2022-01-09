#include <vsntwl.hpp>

#define INCLUDE_SYS_SOCKETS
#include <base.hpp>

#include <stdlib.h>
#include <time.h>
				
void vsntwl::init() {
	srand(time(NULL));
#ifdef _WIN32
	WSAData w_data;
	WSAStartup	(MAKEWORD(2, 2), &w_data);
#endif
}

void vsntwl::deinit() {
#ifdef _WIN32
	WSACleanup();
#endif
}