cmake_minimum_required (VERSION 3.14)

project ("vsntwl")

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRC_FILES "code/*.cpp")
file(GLOB_RECURSE HEADER_FILES "code/*.hpp")

add_library (vsntwl ${SRC_FILES} ${HEADER_FILES})
IF (WIN32)

ENDIF()

IF(UNIX)

ENDIF()