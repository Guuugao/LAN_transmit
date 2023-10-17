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
#define BUFFER_SIZE (1024 * 1024)

// 记录状态的变量
static volatile int status;

// 可能的状态
enum Status{
    leisure     = 0x00000000,   // 空闲
    sending     = 0x00000001,
    receiving   = 0x00000010,
    complete    = 0x00000100,
    error       = 0x00001000,
    cancel      = 0x00010000,
    accept_req  = 0x01000000,
    refuse_req  = 0x00100000,
    interrupt   = 0x10000000
};

#define FILE_PATH_LENGTH 64

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

struct Ack{
    int code;
};