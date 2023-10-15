cmake_minimum_required(VERSION 3.26)
project(LAN_transmit)

# 添加 windows socket 链接库
link_libraries(ws2_32)

set(CMAKE_CXX_STANDARD 17)

add_executable(LAN_transmit main.cpp
        Header/Transmitter_server_TCP.h
        Resource/Transmitter_server_TCP.cpp
        Header/Transmitter_server_TCP.h
        Header/Definitions.h
        Resource/Transmitter_client_TCP.cpp
        Header/Transmitter_client_TCP.h)

# 指定头文件目录
include_directories(${PROJECT_SOURCE_DIR}/Header)