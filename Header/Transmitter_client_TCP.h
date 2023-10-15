//
// Created by TIME LEAP MACHINE on 2023/10/13.
//

#ifndef LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H
#define LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H

#include <thread>
#include <fstream>
#include <iostream>
#include <winsock2.h>

#include "Definitions.h"

// 检测是否正在发送
bool is_sending_object();
// 终止发送
int end_send_object();
// 接收ACK
bool receive_ACK(SOCKET client_sock);
// 开始发送
// TODO 由于istream不能关闭, 所以暂时写成ifstream
int start_send_object(ifstream& data_stream, sockaddr_in server_addr, string& f_name, size_t f_size);

#endif //LAN_TRANSMIT_TRANSMITTER_CLIENT_TCP_H
