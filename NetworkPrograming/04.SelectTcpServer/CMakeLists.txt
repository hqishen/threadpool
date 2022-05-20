cmake_minimum_required(VERSION 3.16)
project(04_SelectTcpServer)

set(CMAKE_CXX_STANDARD 11)

add_executable(04_SelectTcpServer main.cpp)
add_executable(client client.cpp)