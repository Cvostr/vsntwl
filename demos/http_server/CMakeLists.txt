cmake_minimum_required (VERSION 3.14)

project ("demo_http")

set(CMAKE_CXX_STANDARD 17)

include_directories(../../library/headers REQUIRED)

set(SRC_FILES main.cpp http_message.cpp)

add_executable (http_server ${SRC_FILES})
target_link_libraries(http_server vsntwl)	

IF (WIN32)
target_link_libraries(http_server Winmm.lib)
target_link_libraries(http_server Version.lib)
target_link_libraries(http_server Imm32.lib)
target_link_libraries(http_server Bcrypt.lib)
target_link_libraries(http_server ws2_32.lib)
ENDIF()

IF(UNIX)

ENDIF()
