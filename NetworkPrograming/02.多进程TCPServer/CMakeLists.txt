cmake_minimum_required(VERSION 3.16)
project(02_TCPServer)

set(CMAKE_CXX_STANDARD 14)

add_executable(server main.cpp)
add_executable(client client.cpp)