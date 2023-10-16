//
// Created by TIME LEAP MACHINE on 2023/10/13.
//

#ifndef LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H
#define LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H

#include <thread>
#include <vector>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <winsock2.h>

#include "Definitions.h"

// 检测是否正在发送
bool is_sending_object();
// 终止发送
int end_send_object();

// 接收ACK
bool receive_ACK(SOCKET server_sock);
// 发送请求
bool send_request(SOCKET server_sock, string& file_path, struct stat& file_stat);

// 提取文件名
string get_file_name(string& file_path);
// 获取应开辟线程数量
// 可以设计个算法, 不过没啥头绪, 先写死吧
int get_thread_amount(struct stat& file_stat);

// 开始发送
// TODO 按照自己的意思改写一下: 不提供流, 提供文件路径, 函数自行提取文件名&文件流
int start_send_object(sockaddr_in server_addr, string file_path);
// 发送数据块
void send_block(sockaddr_in& server_addr, Block_info& block_info, string& file_path);
#endif //LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H
