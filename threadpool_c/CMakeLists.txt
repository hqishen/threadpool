cmake_minimum_required(VERSION 3.16)
set(BIN_NAME main)
project(${BIN_NAME})
set(CMAKE_CXX_STANDARD 14)

add_executable(${BIN_NAME} main.cpp)
target_link_libraries(${BIN_NAME} pthread)