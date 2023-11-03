#include <iostream>
#include "Transmitter_server_TCP.h"
#include "Transmitter_client_TCP.h"

/*
 * 关于Git推送遇到的问题:
 * git SSL certificate problem: unable to get local issuer certificate
 * 解决方案:
 * 1. 生成SSH秘钥       https://cloud.tencent.com/developer/article/2059781
 * 2. 更改Git url为ssh https://www.cnblogs.com/guobaozhu/p/gitssh.html
 * */

using namespace std;

void startServer(){
    sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(20000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    Transmitter_server_TCP server(server_addr);
    server.start_receive_object("/home/guuugao/code/cpp/LAN_transmit/test_file/");
}

void startClient(){
    sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(20000);
    server_addr.sin_addr.s_addr = inet_addr("172.30.51.195");

    Transmitter_client_TCP client;
    client.start_send_object(server_addr, "/home/guuugao/code/cpp/LAN_transmit/1.mp4");
}

// TODO 考虑使用异常处理
// TODO 断点重传
// TODO 发送文件夹
// TODO 添加日志
int main() {
    thread thread_server(startServer);
    thread thread_client(startClient);

    thread_client.join();
    thread_server.join();

    return 0;
}