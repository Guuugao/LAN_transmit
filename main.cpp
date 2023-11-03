#include <iostream>
#include "Transmitter_server_TCP.h"
#include "Transmitter_client_TCP.h"

/*
 * ����Git��������������:
 * git SSL certificate problem: unable to get local issuer certificate
 * �������:
 * 1. ����SSH��Կ       https://cloud.tencent.com/developer/article/2059781
 * 2. ����Git urlΪssh https://www.cnblogs.com/guobaozhu/p/gitssh.html
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

// TODO ����ʹ���쳣����
// TODO �ϵ��ش�
// TODO �����ļ���
// TODO �����־
int main() {
    thread thread_server(startServer);
    thread thread_client(startClient);

    thread_client.join();
    thread_server.join();

    return 0;
}