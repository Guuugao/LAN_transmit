//
// Created by TIME LEAP MACHINE on 2023/10/8.
//

#ifndef LAN_TRANSMIT_TRANSMITTER_SERVER_TCP_H
#define LAN_TRANSMIT_TRANSMITTER_SERVER_TCP_H

#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <winsock2.h>
#include <unordered_map>

#include "Definitions.h"

class Transmitter_server_TCP{
private:
     std::atomic<enum_state> state;

    // 服务器监听socket
    SOCKET server_sock;
    // 服务器监听地址&端口
    sockaddr_in server_addr;
    // 互斥访问ofs
    std::mutex m_ofs;
    // 写入文件流
    ofstream ofs;
    // 文件信息包
    File_info file_info;
    // 接收线程
    std::unordered_map<SOCKET, thread> sub_thread;


public:
    explicit Transmitter_server_TCP(sockaddr_in server_ad);

    // 是否正在接收文件
    bool is_receiving_object();
    // 结束接收文件
    void end_receive_object();

    // 开始接收文件
    bool start_receive_object(char* save_path);
private:
    // 清理上次接收相关数据
    void clear_member();
    // 发送ACK
    // TODO: 需要用户确认, 暂时先写成控制台确认, 方便调试
    void send_ACK(SOCKET client_sock);

    // 接收各个文件块
    void receive_block(SOCKET sub_sock);
};

#endif //LAN_TRANSMIT_TRANSMITTER_SERVER_TCP_H
