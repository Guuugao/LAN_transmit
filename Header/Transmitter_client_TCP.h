//
// Created by TIME LEAP MACHINE on 2023/10/13.
//

#ifndef LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H
#define LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H

#include <cstring>

#include <mutex>
#include <atomic>
#include <thread>
#include <future>

#include <fstream>
#include <iostream>

#include <algorithm>
#include <unordered_map>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Definitions.h"

class Transmitter_client_TCP{
private:
    // 客户端状态
    volatile std::atomic<enum_state> state;
    // 文件流互斥锁
    std::mutex m_ifs;
    // 读取文件的流
    std::ifstream ifs;

    // 与服务器通信的socket
    socket_fd server_sock;
    // 服务器地址
    sockaddr_in server_addr;
    // 文件信息包
    request_info req_info;
    // 发送线程&对应socket集合
    std::vector<std::future<int>> sub_thread;

public:
    Transmitter_client_TCP();

    // 检测是否正在发送
    bool is_sending_object();
    // 终止发送
    /*
     * 功能: 开始发送
     * 返回值:
     *  0 一切正常
     *  1 发送中, 请等待一段时间尝试
     *  2 网络故障
     *  3 文件相关错误
     *  4 终止
     * */
    void end_send_object();

    int start_send_object(sockaddr_in server_ad, const char* file_path);

private:
    // 清理上次发送相关数据
    void clear_member();

    /*
    * 功能: 接收ACK, 返回接收方是否同意
    * 返回值: 该操作是否成功
    * */
    bool receive_ACK();
    /*
    * 功能: 发送请求
    * 返回值: 该操作是否成功
    * */
    bool send_request();

    // 提取文件名, 传入文件路径分隔符
    const char* get_file_name(const char* file_path, const char* separator);
    // 获取应开辟线程数量
    // 可以设计个算法, 不过没啥头绪, 先写死吧
    int get_thread_amount();
    // 获取文件大小
    long get_file_size();
    /*
    * 功能: 发送数据块
    * 返回值: 同 start_send_object
    * */
    int send_block(block_info block_info);
};

#endif //LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H
