#include <iostream>
#include <sys/stat.h>
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
// �ر��׽��ֺ��ļ���ԭ��: ���Ĵ��������Ĺر�

#define TEST_FILE_NAME "test.mp4"
#define TEST_FILE_PATH "C:\\Users\\TIME LEAP MACHINE\\Downloads\\test.mp4"

void startServer(){
    server_init(INADDR_ANY, SERVER_PORT);
}

void startClient(){
    ifstream ifs(TEST_FILE_PATH, ios::binary | ios::in);
    if (!ifs.is_open()){
        cout << "main: ifs" << endl;
        exit(-1);
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.137.1");

    struct stat file_info;
    stat(TEST_FILE_PATH, &file_info);
    string file_name(TEST_FILE_NAME);
    start_send_object(ifs, server_addr, file_name, file_info.st_size);
}

// TODO ����ʹ���쳣����
int main() {
    // ��windows�����
    WSADATA winSockMsg;
    if (WSAStartup(MAKEWORD(2, 2), &winSockMsg)) {
        cerr << "open windows socket library fail: " << WSAGetLastError() << endl;
        return Status::error;
    }

    // ����汾
    if (2 != HIBYTE(winSockMsg.wVersion) || 2 != LOBYTE(winSockMsg.wVersion)) {
        cerr << "windows socket library version not exists: " << WSAGetLastError() << endl;
        WSACleanup();
        return Status::error;
    }

    thread server(startServer);
    thread client(startClient);

    server.detach();
    client.detach();

    while(true){
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    return 0;
}
