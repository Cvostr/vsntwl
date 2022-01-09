cmake_minimum_required (VERSION 3.14)

project ("vsntwl")

set(CMAKE_CXX_STANDARD 17)

set(SRC_FILES src/Server.cpp
				src/Client.cpp
				src/base.cpp
				src/vsntwl.cpp
				src/IPAddress.cpp)
set(HEADER headers/Server.hpp
			headers/Client.hpp
			headers/base.hpp
			headers/vsntwl.hpp
			headers/IPAddress.hpp)

include_directories("./headers" REQUIRED)

add_library (vsntwl ${SRC_FILES} ${HEADER_FILES})