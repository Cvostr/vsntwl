#include <Socket.hpp>

#ifdef __linux__
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

int vsntwl::CloseSocket(SOCKET socket){
#ifdef _WIN32
    return ::closesocket(socket);
#endif

#ifdef __linux__
    return ::close(socket);
#endif
}

int vsntwl::GetLastSockErrCode(){
#ifdef _WIN32
    return ::WSAGetLastError();
#endif

#ifdef __linux__
    return errno;
#endif
}

int vsntwl::DisableBlocking(SOCKET socket){
#ifdef _WIN32
    u_long iMode = 1;
    return ::ioctlsocket(socket, FIONBIO, &iMode);
#endif

#ifdef __linux__
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) 
        return false;
    flags |= O_NONBLOCK;
    int result = fcntl(socket, F_SETFL, flags);

    //int imode = 1;
    //ioctl(socket , FIONBIO, &imode);
    return result;
#endif
}

void vsntwl::FillInaddrStruct(const IPAddress4& address, unsigned short port, sockaddr_in& out){
#ifdef _WIN32
    out.sin_addr.S_un.S_addr = address.ip;
#endif

#ifdef __linux__
    out.sin_addr.s_addr = address.ip;
#endif
	out.sin_port = htons(port); //Setting port
	out.sin_family = AF_INET; //IPv4 addresses
}

void vsntwl::FillInaddrStruct(unsigned short port, sockaddr_in& out){
#ifdef _WIN32
    out.sin_addr.S_un.S_addr = INADDR_ANY; //Any IP address
#endif
#ifdef __linux__
    out.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
	out.sin_port = htons(port); //Setting port
	out.sin_family = AF_INET; //IPv4 addresses
}

unsigned int vsntwl::GetAddressInteger(const sockaddr_in& in){
#ifdef _WIN32
    return in.sin_addr.S_un.S_addr;
#endif
#ifdef __linux__
    return in.sin_addr.s_addr;
#endif
}

SOCKET vsntwl::AcceptSocket(SOCKET socket, sockaddr_in& in){
#ifdef _WIN32
    int addrlen = sizeof(sockaddr_in);
#endif
#ifdef __linux__
    unsigned int addrlen = sizeof(sockaddr_in);
#endif
    return ::accept(socket, (struct sockaddr*)&in, &addrlen);
}

int vsntwl::RecvFrom(SOCKET socket, char* buffer, unsigned int size, sockaddr_in& sender){
#ifdef _WIN32
    int addrlen = sizeof(sockaddr_in);
#endif
#ifdef __linux__
    unsigned int addrlen = sizeof(sockaddr_in);
#endif
    return ::recvfrom(socket, buffer, DEFAULT_BUFLEN, 0, (struct sockaddr*)&sender, &addrlen);
}

int vsntwl::SendTo(SOCKET socket, const char* buffer, unsigned int size, const sockaddr_in& dest) {
#ifdef _WIN32
    int addrlen = sizeof(sockaddr_in);
#endif
#ifdef __linux__
    unsigned int addrlen = sizeof(sockaddr_in);
#endif
    return ::sendto(socket, buffer, size, 0, (const sockaddr*)&dest, addrlen);
}