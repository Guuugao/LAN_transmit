//
// Created by TIME LEAP MACHINE on 2023/10/13.
//

#ifndef LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H
#define LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H

#include <atomic>
#include <thread>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <winsock2.h>
#include <unordered_map>

#include "Definitions.h"

class Transmitter_client_TCP{
private:
    // 客户端状态
    std::atomic<enum_state> state;

    // 与服务器通信的socket
    SOCKET server_sock;
    // 服务器地址
    sockaddr_in server_addr;
    // 文件信息包
    File_info file_info;
    // 发送线程&对应socket集合
    std::unordered_map<SOCKET, thread> sub_thread;

public:
    Transmitter_client_TCP();

    // 检测是否正在发送
    bool is_sending_object();
    // 终止发送
    void end_send_object();

    // 开始发送
    // TODO 按照自己的意思改写一下: 不提供流, 提供文件路径, 函数自行提取文件名&文件流
    bool start_send_object(sockaddr_in server_ad, const char* file_path);

private:
    // 清理上次发送相关数据
    void clear_member();
    // 接收ACK, 返回接收方是否同意
    bool receive_ACK();
    // 发送请求
    void send_request();

    // 提取文件名, 传入文件路径分隔符
    const char* get_file_name(const char* file_path, const char* separator);
    // 获取应开辟线程数量
    // 可以设计个算法, 不过没啥头绪, 先写死吧
    int get_thread_amount();
    // 获取文件大小
    u_int64 get_file_size(const char* file_path);

    // 发送数据块
    void send_block(SOCKET sub_sock, Block_info block_info, const char* file_path);
};

#endif //LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H
