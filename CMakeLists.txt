cmake_minimum_required(VERSION 3.26)
project(LAN_transmit)

set(CMAKE_CXX_STANDARD 17)

# Build for Common Language Runtime.

add_library(lan_transmit SHARED
        Header/Transmitter_server_TCP.h
        Header/Definitions.h
        Header/Transmitter_client_TCP.h
        Resource/Transmitter_server_TCP.cpp
        Resource/Transmitter_client_TCP.cpp
        Resource/Export.cpp
)

# set_target_properties(lan_transmit PROPERTIES COMMON_LANGUAGE_RUNTIME "")

# Build for test

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