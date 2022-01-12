#include <IPAddress.hpp>
#include <sstream>
#include <string.h>

using namespace vsntwl;

IPAddress4::IPAddress4() {
	_0 = 127;
	_1 = 0;
	_2 = 0;
	_3 = 1;
}
IPAddress4::IPAddress4(const std::string& str) {
	std::istringstream f(str);
	std::string s;
	int i = 0;
	while (getline(f, s, '.')) {
		if (i == 4)
			break;
		ip_a[i++] = atoi(s.c_str());
	}
}
IPAddress4::IPAddress4(unsigned int ip) {
	this->ip = ip;
}
IPAddress4::IPAddress4(unsigned char s0, unsigned char s1, unsigned char s2, unsigned char s3) {
	_0 = s0;
	_1 = s1;
	_2 = s2;
	_3 = s3;
}

IPAddress4::IPAddress4(const IPAddress4& ip) {
	this->ip = ip.ip;
}

IPAddress4& IPAddress4::operator=(const IPAddress4& ip) {
	this->ip = ip.ip;
	return *this;
}

bool IPAddress4::operator==(const IPAddress4& ip) const {
	return ip.ip == this->ip;
}