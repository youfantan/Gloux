cmake_minimum_required(VERSION 3.24)
project(gloux)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -pthread)
include_directories(include)
add_library(gloux-network include/gloux/network.h network/linux/glx_socket.cpp include/gloux/log.h include/gloux/gloux.h)
add_executable(gloux-network-test tests/network_test.cpp)
target_link_libraries(gloux-network-test gloux-network)