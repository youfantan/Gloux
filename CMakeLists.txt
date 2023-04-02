cmake_minimum_required(VERSION 3.24)
project(gloux)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -pthread)
include_directories(include)
add_executable(gloux-network-test tests/network_test.cpp)
add_executable(gloux-io-test tests/io_test.cpp)