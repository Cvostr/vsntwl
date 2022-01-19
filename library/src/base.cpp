#define INCLUDE_SYS_SOCKETS
#include <base.hpp>
#include <ctime>
#include <chrono>
#include <thread>

static unsigned int rand_seed = time(0);

int ConvertProtocol(InetProtocol protocol) {
	int result = 0;
	switch (protocol) {
	case INET_PROTOCOL_TCP:
		result = SOCK_STREAM;
		break;
	case INET_PROTOCOL_UDP:
		result = SOCK_DGRAM;
		break;
	}
	return result;
}

unsigned int get_random_value() {
	rand_seed = rand_seed * 214013 + 2531011;
	return (rand_seed >> 16) & 32767u; //32767;
}

unsigned int get_random_value(unsigned int min, unsigned int max) {
	return min + get_random_value() % (max - min);
}

unsigned int get_current_time_ms() {
	const auto p1 = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count();
}

void sleep_cur_thread_ms(unsigned int time) {
	std::this_thread::sleep_for(std::chrono::milliseconds(time));
}