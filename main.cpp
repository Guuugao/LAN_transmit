#include <iostream>
#include <sys/stat.h>
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
// 关闭套接字和文件流原则: 在哪创建就在哪关闭

#define TEST_FILE_PATH_SMALL "C:\\Users\\TIME LEAP MACHINE\\Downloads\\test_0.mp4"
#define TEST_FILE_PATH_BIG "C:\\Users\\TIME LEAP MACHINE\\Downloads\\test_1.mp4"

void startServer(){
    sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.S_un.S_addr = ADDR_ANY;

    Transmitter_server_TCP server(server_addr);
    server.start_receive_object();
    server.start_receive_object();
}

void startClient(){
    sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.137.1");

    Transmitter_client_TCP client;
    client.start_send_object(server_addr, TEST_FILE_PATH_BIG);
    client.start_send_object(server_addr, TEST_FILE_PATH_SMALL);
}

// TODO 考虑使用异常处理
int main() {
    // 打开windows网络库
    WSADATA winSockMsg;
    if (WSAStartup(MAKEWORD(2, 2), &winSockMsg)) {
        cerr << "open windows socket library fail: " << WSAGetLastError() << endl;
        return Status::error;
    }

    // 检验版本
    if (2 != HIBYTE(winSockMsg.wVersion) || 2 != LOBYTE(winSockMsg.wVersion)) {
        cerr << "windows socket library version not exists: " << WSAGetLastError() << endl;
        WSACleanup();
        return Status::error;
    }

    thread thread_client(startClient);
    thread thread_server(startServer);

    thread_client.detach();
    thread_server.detach();

    while(true){
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    return 0;
}
