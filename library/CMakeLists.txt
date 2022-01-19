cmake_minimum_required (VERSION 3.14)

project ("vsntwl")

set(CMAKE_CXX_STANDARD 17)

set(SRC_FILES 	
				src/ConnectedClient.cpp
				src/Socket.cpp
				src/Server.cpp
				src/TCPServer.cpp
				src/UDPServer.cpp
				src/Client.cpp
				src/TCPClient.cpp
				src/UDPClient.cpp
				src/base.cpp
				src/vsntwl.cpp
				src/IPAddress.cpp)
set(HEADER 	
			headers/ConnectedClient.hpp
			headers/Socket.hpp
			headers/Server.hpp
			headers/TCPServer.hpp
			headers/UDPServer.hpp
			headers/Client.hpp
			headers/TCPClient.hpp
			headers/UDPCient.hpp
			headers/base.hpp
			headers/vsntwl.hpp
			headers/IPAddress.hpp)

include_directories("./headers" REQUIRED)

add_library (vsntwl ${SRC_FILES} ${HEADER_FILES})