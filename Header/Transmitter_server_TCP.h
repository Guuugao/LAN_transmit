//
// Created by TIME LEAP MACHINE on 2023/10/8.
//

#ifndef LAN_TRANSMIT_TRANSMITTER_SERVER_TCP_H
#define LAN_TRANSMIT_TRANSMITTER_SERVER_TCP_H

#include <cstring>

#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <future>

#include <fstream>
#include <iostream>

#include <vector>

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "Definitions.h"

class Transmitter_server_TCP{
private:
    // server status
    volatile std::atomic<enum_state> state;
    // output file stream lock
    std::mutex m_ofs;
    std::ofstream ofs;

    // 服务器监听socket
    socket_fd server_sock;
    // 服务器监听地址&端口
    sockaddr_in server_addr;
    // 文件信息包
    request_info req_info;
    // 地址结构体长度
    socklen_t addr_len;
    // 接收线程集合
    std::vector<std::future<int>> sub_thread;

    std::promise<int> pms_server;
public:
    explicit Transmitter_server_TCP(sockaddr_in server_ad);
    ~Transmitter_server_TCP();

    // 是否正在接收文件
    bool is_receiving_object();
    // 结束接收文件
    void end_receive_object();

    /*
     * 功能: 接收文件前置工作
     * 返回值:
     *  0 一切正常
     *  1 接收中, 请等待一段时间尝试
     *  2 网络故障
     *  3 文件相关错误
     *  4 终止
     *  5 未知错误
     * */
    // TODO 存在重名文件需要更改文件名称
    // TODO 文件名和保存路径之间需要有一个目录分隔符, 不同系统不一样
    std::future<int> start_receive_object(const char* save_path);
private:
    // 接收文件
    void do_receive();

    // 接收各个文件块
    int receive_block();

    // 清理上次接收相关数据
    void clear_member();
    // 发送ACK, 返回是否发送成功
    // TODO: 需要用户确认, 暂时先写成控制台确认, 方便调试
    bool send_ACK(socket_fd fd, int code);
};

#endif //LAN_TRANSMIT_TRANSMITTER_SERVER_TCP_H
