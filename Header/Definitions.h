//
// Created by TIME LEAP MACHINE on 2023/10/8.
//
#pragma once

using std::ios;
using std::cout;
using std::cerr;
using std::endl;
using std::thread;
using std::string;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;

using u_llong = unsigned long long;

#define DEFAULT_SERVER_PORT (9000)
#define DEFAULT_SERVER_ADDR "0.0.0.0"

#define SOCKADDR_CAST(addr) reinterpret_cast<sockaddr*>(&addr)
#define CHAR_POINTER_CAST(obj) reinterpret_cast<char*>(&obj)

/* 数据包相关定义 */
// 一个文件块的大小 -- 1M
#define BUFFER_SIZE (1024 * 1024)

// 表示当前状态
enum Status{
    sending     = 0x00000001,
    receiving   = 0x00000010,
    complete    = 0x00000100,
    error       = 0x00001000,
    cancel      = 0x00010000,
    accept_req  = 0x01000000,
    refuse_req  = 0x00100000,
    interrupt   = 0x10000000
};

#define FILE_NAME_LENGTH 64
// 文件信息(文件名称, 文件大小)
struct File_info{
    char file_name[FILE_NAME_LENGTH];
    u_llong file_size;
};

// 文件数据块信息
struct Data_block_info{
    u_llong seek; // 文件偏移量
    u_llong size; // 数据块大小
};

struct ACK{
    int code;
};