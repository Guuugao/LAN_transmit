//
// Created by TIME LEAP MACHINE on 2023/10/8.
//
#pragma once

using std::ios;
using std::ref;
using std::cout;
using std::cerr;
using std::endl;
using std::thread;
using std::string;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;

#define SERVER_PORT (9000)
/* 数据包相关定义 */
// 一个文件块的大小 -- 1M
#define BUFFER_SIZE (1 << 11)

// 可能的状态
enum enum_state{
    leisure     = 0x00000000,   // 空闲
    sending     = 0x00000001,
    receiving   = 0x00000010,
};

// 文件信息(文件名称, 文件大小)
struct File_info{
    const char* file_name;   // 文件名称
    u_int64 file_size;  // 文件大小
    u_int thread_amount;// 计划使用线程数量
};

// 文件数据块信息
struct Block_info{
    u_int64 seek; // 文件偏移量
    u_int64 size; // 数据块大小
};


#define ACCEPT_REQUEST 1
#define REFUSE_REQUEST 0

struct Ack{
    int code;
};