cmake_minimum_required (VERSION 3.14)

project ("demo_client")

set(CMAKE_CXX_STANDARD 17)

include_directories(../../library/headers REQUIRED)

set(SRC_FILES main.cpp)

add_executable (demo_client ${SRC_FILES})
target_link_libraries(demo_client vsntwl)	

IF (WIN32)
target_link_libraries(demo_client Winmm.lib)
target_link_libraries(demo_client Version.lib)
target_link_libraries(demo_client Imm32.lib)
target_link_libraries(demo_client Bcrypt.lib)
target_link_libraries(demo_client ws2_32.lib)
ENDIF()

IF(UNIX)

ENDIF()
