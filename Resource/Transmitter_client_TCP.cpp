//
// Created by TIME LEAP MACHINE on 2023/10/13.
//
#include <vector>
#include "Transmitter_client_TCP.h"
#include "Definitions.h"
bool Transmitter_client_TCP::is_sending_object() {
    return false;
}

void Transmitter_client_TCP::end_send_object() {

}

int Transmitter_client_TCP::start_send_object(sockaddr_in server_ad, const char *file_path) {
    clear_member();

    file_info.file_name = get_file_name(file_path, "/\\");
    file_info.file_size = get_file_size(file_path);
    file_info.thread_amount = get_thread_amount();

    server_addr = server_ad;
    u_int64 block_size = (file_info.file_size / file_info.thread_amount) + 1;
    sub_thread.reserve(file_info.thread_amount);

    if (strlen(file_path) > FILE_PATH_LENGTH) {
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

    send_request();
    cout << "client: send file information" << endl;

    if (receive_ACK()){
        closesocket(server_sock); // 允许发送, 可以关闭先前的连接
        for (int i = 0; i < file_info.thread_amount; ++i) {
            SOCKET sub_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            Block_info block_info = { 0 };
            // 最后一个线程, 块大小不一定等于定义的块大小
            block_info.size = (i == (file_info.thread_amount - 1)) ? (file_info.file_size % block_size) : block_size;
            block_info.seek = block_size * i;

            cout << "block " << i << " size: " << block_info.size << endl;

            sub_thread.emplace(sub_sock, thread(&Transmitter_client_TCP::send_block, this,
                                                sub_sock, block_info, file_path));
        }
        for (auto &item: sub_thread){
            if (item.second.joinable()) item.second.join();
        }
    }
    closesocket(server_sock);
    cout << "client: send complete" << endl;
    return Status::complete;
}


void Transmitter_client_TCP::send_block(SOCKET sub_sock, Block_info block_info, const char *file_path) {
    u_int bytes = 0;        // 单次实际发送的字节数
    u_int send_size = 0;    // 单次应该发送的字节数
    u_int total_bytes = 0;  // 总计已发送字节数
    char buff[BUFFER_SIZE];
    ifstream ifs(file_path, ios::in | ios::binary);
    ifs.seekg(block_info.seek);

    if(connect(sub_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        printf("ERR client: sub_thread connect server %d\n", WSAGetLastError());
        return;
    }
    printf("client: sub_thread connect server\n");

    if (send(sub_sock, reinterpret_cast<char*>(&block_info), sizeof(Block_info), 0) == SOCKET_ERROR){
        printf("ERR client: sub_thread send block info %d\n", WSAGetLastError());
        return;
    }
    printf("client: sub_thread connect server\n");

    while (total_bytes < block_info.size) {
        send_size = std::min((block_info.size - total_bytes), static_cast<u_int64>(BUFFER_SIZE));
        ifs.read(buff, send_size).gcount();
        bytes = send(sub_sock, buff, send_size, 0);
        if (bytes == -1) {
            printf("ERR client: sub_thread send block %d\n", WSAGetLastError());
            break;
        }
        printf("client: sub_thread send block\n");

        total_bytes += bytes;
    }

    printf("client: sub_thread send block complete\n");
    ifs.close();
    closesocket(sub_sock);
}

void Transmitter_client_TCP::clear_member() {
    server_sock = INVALID_SOCKET;
    memset(&file_info, 0, sizeof (File_info));
    memset(&server_addr, 0, sizeof (sockaddr_in));
    sub_thread.clear();
}

bool Transmitter_client_TCP::receive_ACK() {
    Ack ack = { 0 };
    if (recv(server_sock, reinterpret_cast<char*>(&ack), sizeof (Ack), 0) == SOCKET_ERROR) {
        cerr << "client: receive Ack " << WSAGetLastError() << endl;
    }
    cout << "client: received Ack" << endl;
    return ack.code == Status::accept_req;
}

void Transmitter_client_TCP::send_request(){
    // TODO 发送失败处理
    send(server_sock, reinterpret_cast<char*>(&file_info),sizeof (File_info), 0);
}

const char *Transmitter_client_TCP::get_file_name(const char* file_path, const char* separator) {
    const char* s = file_path + strlen(file_path) - 1; // 从后往前查找
    while (s != file_path){
        const char *a = separator; // 依次比较每个分隔符
        while (*a != '\0')
            if (*a++ == *s)
                return ++s;
        --s;
    }
    return nullptr;
}

int Transmitter_client_TCP::get_thread_amount(){
    if (file_info.file_size <= (1ULL << 28)) { // 文件大小 <= 256M
        return 2;
    } else if (file_info.file_size <= (1ULL << 31)) { // 文件大小 <= 2G
        return 4;
    } else {
        return 8;
    }
}

u_int64 Transmitter_client_TCP::get_file_size(const char* file_path) {
    ifstream ifs(file_path, ios::in | ios::binary);
    ifs.seekg(0, ios::end);
    u_int64 len = ifs.tellg();

    ifs.close();
    return len;
}

Transmitter_client_TCP::Transmitter_client_TCP() {
    clear_member();
}
