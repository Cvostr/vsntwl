#pragma once

#include <base.hpp>
#include <string>

namespace vsntwl {
	class IPAddress4 {
	public:
		union {
			struct {
				unsigned char _0;
				unsigned char _1;
				unsigned char _2;
				unsigned char _3;
			};
			unsigned int ip;
			unsigned char ip_a[4];
		};

		IPAddress4();
		IPAddress4(const std::string& str);
		IPAddress4(unsigned int ip);
		IPAddress4(unsigned char s0, unsigned char s1, unsigned char s2, unsigned char s3);

		IPAddress4(const IPAddress4& ip);
		IPAddress4& operator=(const IPAddress4& ip);
		bool operator==(const IPAddress4& ip) const;
	};
}