//
// Created by TIME LEAP MACHINE on 2023/10/8.
//
#pragma once

typedef int socket_fd;

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

/* 数据包相关定义 */
// 一个文件块的大小 -- 1M
#define BUFFER_SIZE (1 << 19)

// 可能的状态
enum enum_state{
    leisure     = 0x00000000,
    sending     = 0x00000001,
    receiving   = 0x00000010,
    interrupt   = 0x00000100,
    error       = 0x00001000,
};

// 文件信息(文件名称, 文件大小)
struct request_info{
    long file_size;  // 文件大小
    const char* file_name;   // 文件名称
    u_int thread_amount;// 计划使用线程数量
};

// 文件数据块信息
struct block_info{
    long seek; // 文件偏移量
    long size; // 数据块大小
};

// 1: 接受
// 0: 拒绝
struct ack_info{
    int code;
};