//
// Created by TIME LEAP MACHINE on 2023/10/8.
//

#include "Transmitter_server_TCP.h"

bool Transmitter_server_TCP::is_receiving_object() {
    return false;
}

void Transmitter_server_TCP::end_receive_object() {

}

bool Transmitter_server_TCP::start_receive_object(char* save_path) {
    if (state != enum_state::leisure) return false;
    state.store(enum_state::receiving);

    sockaddr_in client_addr = { 0 };
    int addr_len = sizeof(sockaddr);
    SOCKET client_sock = accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);

    if (recv(client_sock, reinterpret_cast<char *>(&file_info), sizeof(File_info), 0) == SOCKET_ERROR) {
        cerr << "server: receive file information  " << WSAGetLastError() << endl;
        return false;
    }
    // 拼接存储路径与文件名
    const char* file_path = strcat(save_path, file_info.file_name);
    sub_thread.reserve(file_info.thread_amount);
    ofs.open(file_path, ios::binary | ios::out | ios::trunc);
    if (!ofs.is_open()) {
        cerr << "server: create file " << endl;
        return false;
    }

    send_ACK(client_sock);
    closesocket(client_sock);

    for (int i = 0; i < file_info.thread_amount; ++i) {
        SOCKET sub_sock = accept(server_sock, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
        sub_thread.emplace(sub_sock, thread(&Transmitter_server_TCP::receive_block, this, sub_sock));
    }

    for (auto &item: sub_thread) {
        if (item.second.joinable()) item.second.join();
    }

    ofs.close();
    ofs.clear();
    cout << "server: receive complete" << endl;
    state.store(enum_state::leisure);
    return true;
}

void Transmitter_server_TCP::receive_block(SOCKET sub_sock) {
    u_int bytes; // 单次接收的字节数
    u_int total_bytes = 0; // 记录已经接收到的文件字节数
    char buff[BUFFER_SIZE];
    Block_info block_info;

    if (recv(sub_sock, reinterpret_cast<char *>(&block_info), sizeof(Block_info), 0) == -1) {
        printf("ERR server: sub_thread receive block info %d\n", WSAGetLastError());
        return;
    }

    while (total_bytes < block_info.size) {
        bytes = recv(sub_sock, buff, BUFFER_SIZE, 0);
        if (bytes == -1) {
            printf("ERR server: sub_thread receive block %d\n", WSAGetLastError());
        }

        std::lock_guard<std::mutex> lg_m_ofs(m_ofs); // 访问临界区
        ofs.seekp(block_info.seek + total_bytes); // 设置数据填充位置
        ofs.write(buff, bytes);
        total_bytes += bytes;
    }
    printf("server: sub_thread receive block complete\n");
    closesocket(sub_sock);
}

void Transmitter_server_TCP::clear_member() {
    state.store(enum_state::leisure);
    server_sock = INVALID_SOCKET;
    memset(&file_info, 0, sizeof(File_info));
    memset(&server_addr, 0, sizeof(sockaddr_in));
    // 关于流 clear 与 close 调用顺序的讨论:
    // https://bbs.csdn.net/topics/260042059
    ofs.close();
    ofs.clear();
    sub_thread.clear();
}

void Transmitter_server_TCP::send_ACK(SOCKET client_sock) {
    Ack ack = { 0 };
    ack.code = ACCEPT_REQUEST; // 先默认接受, 方便测试
    int bytes = send(client_sock, reinterpret_cast<char *>(&ack), sizeof(Ack), 0);
    if (bytes == -1) {
        cerr << "server: send Ack " << WSAGetLastError() << endl;
    }
}

// TODO 错误处理
Transmitter_server_TCP::Transmitter_server_TCP(sockaddr_in server_ad) {
    clear_member();
    server_addr = server_ad;

    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock == INVALID_SOCKET) {
        cerr << "server: open server socket fail: " << WSAGetLastError() << endl;
    }

    if (bind(server_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        std::cerr << "server: bind address fail: " << WSAGetLastError() << std::endl;
    }

    if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "server: cant listen request: " << WSAGetLastError() << std::endl;
    }
}