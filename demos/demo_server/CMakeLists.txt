cmake_minimum_required (VERSION 3.14)

project ("demo_server")

set(CMAKE_CXX_STANDARD 17)

include_directories(../../library/headers REQUIRED)

set(SRC_FILES main.cpp)

add_executable (demo_server ${SRC_FILES})
target_link_libraries(demo_server vsntwl)	

IF (WIN32)
target_link_libraries(demo_server Winmm.lib)
target_link_libraries(demo_server Version.lib)
target_link_libraries(demo_server Imm32.lib)
target_link_libraries(demo_server Bcrypt.lib)
target_link_libraries(demo_server ws2_32.lib)
ENDIF()

IF(UNIX)

ENDIF()
