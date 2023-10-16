//
// Created by TIME LEAP MACHINE on 2023/10/13.
//
#include <vector>
#include "Transmitter_client_TCP.h"
#include "Definitions.h"

bool is_sending_object() {
    return true;
}

int end_send_object() {
    return 0;
}

int start_send_object(sockaddr_in server_addr, string file_path) {
    SOCKET server_sock = SOCKET_ERROR;
    std::vector<thread> threads_uploader;
    struct stat file_stat { 0 }; // 获取文件信息
    stat(file_path.c_str(), &file_stat);
    u_int thread_amount = get_thread_amount(file_stat);
    u_int block_size = (file_stat.st_size / thread_amount) + 1;

    threads_uploader.reserve(thread_amount);

    if (file_path.size() > FILE_PATH_LENGTH) {
        cerr << "client: file name too long" << endl;
        return Status::error;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_sock == INVALID_SOCKET) {
        cerr << "client: open socket " << WSAGetLastError() << endl;
        return Status::error;
    }
    cout << "client: open socket" << endl;

    if(connect(server_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        cerr << "client: connect server " << WSAGetLastError() << endl;
        return Status::error;
    }
    cout << "client: connect server" << endl;

    if (!send_request(server_sock, file_path, file_stat)){
        cerr << "client: send file information " << WSAGetLastError() << endl;
        return Status::error;
    }
    cout << "client: send file information" << endl;
    closesocket(server_sock);

    if (receive_ACK(server_sock)){
        for (int i = 0; i < thread_amount; ++i) {
            Block_info block_info;
            // 最后一个线程, 块大小不一定等于定义的块大小
            block_info.size = i == (thread_amount - 1) ? (file_stat.st_size % block_size) : block_size;
            block_info.seek = block_size * i;
            threads_uploader.emplace_back(send_block,
                             ref(server_addr), ref(block_info), ref(file_path));
        }
    }
    cout << "client: send complete" << endl;
    return Status::complete;
}

// TODO 各种异常情况的处理
void send_block(sockaddr_in &server_addr, Block_info &block_info, string &file_path) {
    u_int bytes = 0;    // 单次发送的字节数
    u_int totle_bytes = 0; // 总计已发送字节数
    char buff[BUFFER_SIZE];
    ifstream ifs(file_path, ios::in | ios::binary);

    SOCKET sub_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sub_sock == INVALID_SOCKET) {
        cerr << "client: sub_thread open socket " << WSAGetLastError() << endl;
    }
    cout << "client: open socket" << endl;

    if(connect(sub_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        cerr << "client: connect server " << WSAGetLastError() << endl;
    }
    cout << "client: connect server" << endl;

    while (totle_bytes < block_info.size) {
        ifs.read(buff, BUFFER_SIZE).gcount();
    }
}

bool send_request(SOCKET server_sock, string& file_path, struct stat& file_stat) {
    File_info file_info;
    file_info.file_name = get_file_name(file_path);
    file_info.file_size = file_stat.st_size;
    file_info.thread_amount = get_thread_amount(file_stat);

    return send(server_sock, reinterpret_cast<char*>(&file_info),
                sizeof (File_info), 0) != SOCKET_ERROR;
}

bool receive_ACK(SOCKET server_sock) {
    ACK ack = { 0 };
    ack.code = Status::accept_req;
    if (recv(server_sock, reinterpret_cast<char*>(&ack), sizeof (ACK), 0) == SOCKET_ERROR) {
        cerr << "client: receive ACK " << WSAGetLastError() << endl;
    }
    cout << "client: received ACK" << endl;
    return ack.code == Status::accept_req;
}

string get_file_name(string &file_path) {
    u_int pos = file_path.find_last_of("/\\");
    return file_path.substr( + 1, file_path.size() - 1 - pos);
}

int get_thread_amount(struct stat& file_stat) {
    if (file_stat.st_size <= (1 << 8)) { // 文件大小 <= 256M
        return 2;
    } else if (file_stat.st_size <= (1 << 10)) { // 文件大小 <= 2G
        return 4;
    } else { // 文件大小 <= 4G
        return 8;
    }
}

