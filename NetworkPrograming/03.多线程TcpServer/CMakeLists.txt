cmake_minimum_required(VERSION 3.16)
project(03_TcpServer)

set(CMAKE_CXX_STANDARD 14)

add_executable(03_TcpServer main.cpp)
target_link_libraries(03_TcpServer pthread)

add_executable(client client.cpp)
#target_link_libraries(client pthread)