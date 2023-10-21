cmake_minimum_required(VERSION 3.26)
project(LAN_transmit)

# 添加 windows socket 链接库
link_libraries(ws2_32)

set(CMAKE_CXX_STANDARD 17)

add_library(lan_transmit SHARED
        Header/Transmitter_server_TCP.h
        Header/Definitions.h
        Header/Transmitter_client_TCP.h
        Resource/Transmitter_server_TCP.cpp
        Resource/Transmitter_client_TCP.cpp
        Resource/Export.cpp
)

add_library(lan_transmit_static_test
        Header/Transmitter_server_TCP.h
        Header/Definitions.h
        Header/Transmitter_client_TCP.h
        Resource/Transmitter_server_TCP.cpp
        Resource/Transmitter_client_TCP.cpp
)

add_executable(Lan_transmit_test main.cpp)

target_link_libraries(Lan_transmit_test lan_transmit_static_test)

# 指定头文件目录
include_directories(${PROJECT_SOURCE_DIR}/Header)