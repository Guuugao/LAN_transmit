//
// Created by TIME LEAP MACHINE on 2023/10/8.
//

#ifndef LAN_TRANSMIT_TRANSMITTER_SERVER_TCP_H
#define LAN_TRANSMIT_TRANSMITTER_SERVER_TCP_H

#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <winsock2.h>

#include "Definitions.h"

// 等待请求
void wait_request(SOCKET host_sock);
// 发送ACK
bool send_ACK(SOCKET client_sock);
// 开始接收文件
int start_receive_object(SOCKET client_sock);




// 传入指定地址和端口号, 作为服务器地址与监听端口
int server_init(u_long addr, int port);

// 是否正在接收文件
bool is_receiving_object();
// 结束接收文件
int end_receive_object();

#endif //LAN_TRANSMIT_TRANSMITTER_SERVER_TCP_H
