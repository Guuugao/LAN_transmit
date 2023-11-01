//
// Created by TIME LEAP MACHINE on 2023/10/8.
//

#include "Transmitter_server_TCP.h"

bool Transmitter_server_TCP::is_receiving_object() {
    return state == enum_state::receiving;
}

void Transmitter_server_TCP::end_receive_object() {
    state = enum_state::interrupt;
}

int Transmitter_server_TCP::start_receive_object(const char *save_path) {
    if (state != enum_state::leisure) return 1;
    state.store(enum_state::receiving);
    clear_member();

    sockaddr_in client_addr = {0};
    socket_fd client_sock = accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);

    if (recv(client_sock, reinterpret_cast<char *>(&req_info), sizeof(request_info), 0) <= 0) {
        cerr << "server: receive file information" << endl;
        return 2;
    }

    printf("server: request info: %s %ld %d\n", req_info.file_name, req_info.file_size, req_info.thread_amount);

    // 拼接存储路径与文件名
    char* save_path_copy = new char[strlen(save_path) + 128];
    strcpy(save_path_copy, save_path);
    save_path_copy = strcat(save_path_copy, req_info.file_name);

    sub_thread.reserve(req_info.thread_amount);
    ofs.open(save_path_copy, ios::out | ios::binary);
    delete[] save_path_copy;
    if (!ofs.good()) {
        cerr << "server: create file " << endl;
        ofs.close();
        ofs.clear();
        close(client_sock);
        return 3;
    }

    if (!send_ACK(client_sock)) {
        cerr << "server: send ack" << endl;
        ofs.close();
        ofs.clear();
        close(client_sock);
        return 2;
    }

    close(client_sock);
    for (int i = 0; i < req_info.thread_amount; ++i) {
        sub_thread.emplace_back(std::async(&Transmitter_server_TCP::receive_block, this, ref(client_addr)));
    }

    int sub_thread_rtn = 0;
    for (auto &item: sub_thread) {
        int res = item.get();
        if (res != 0) sub_thread_rtn = res;

        printf("server sub thread res: %d\n", res);
    }

    ofs.close();
    ofs.clear();
    cout << "server: receive complete" << endl;
    state.store(enum_state::leisure);
    return sub_thread_rtn;
}

int Transmitter_server_TCP::receive_block(sockaddr_in &client_addr) {
    long bytes = 0;            // 单次接收的字节数
    long total_bytes = 0;  // 记录已经接收到的文件字节数
    char buff[BUFFER_SIZE] = { 0 };
    block_info blk_info = { 0 };
    // TODO 这里没有加锁不知道会不会有问题...
    socket_fd sub_sock = accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);

    if (sub_sock == INVALID_SOCKET) {
        printf("ERR server: sub_thread open socket\n");
        state = enum_state::error;
        return 2;
    }

    printf("server sub socket: %d\n", sub_sock);

    if (recv(sub_sock, reinterpret_cast<char *>(&blk_info), sizeof(block_info), 0) <= 0) {
        printf("ERR server: sub_thread receive block info\n");
        state = enum_state::error;
        close(sub_sock);
        return 2;
    }

    printf("server: block info: %ld %ld\n", blk_info.size, blk_info.seek);

    while (total_bytes < blk_info.size) {
        std::lock_guard<std::mutex> lg_m_ofs(m_ofs); // 访问临界区

        if (state != enum_state::receiving){
            close(sub_sock);
            if (state == enum_state::error) return 2;
            else if (state == enum_state::interrupt) return 4;
            else return 5;
        }

        bytes = recv(sub_sock, buff, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            printf("ERR server: sub_thread receive block\n");
            state = enum_state::error;
            close(sub_sock);
            return 2;
        }

        ofs.seekp(blk_info.seek + total_bytes); // 设置数据填充位置
        ofs.write(buff, bytes);
        total_bytes += bytes;
    }
    printf("server: sub_thread receive block complete\n");
    close(sub_sock);
    return 0;
}

void Transmitter_server_TCP::clear_member() {
    memset(&req_info, 0, sizeof(request_info));
    memset(&server_addr, 0, sizeof(sockaddr_in));
    // 关于流 clear 与 close 调用顺序的讨论:
    // https://bbs.csdn.net/topics/260042059
    sub_thread.clear();
    ofs.close();
    ofs.clear();
}

bool Transmitter_server_TCP::send_ACK(socket_fd client_sock) {
    ack_info ack = {0};
    ack.code = 1; // 先默认接受, 方便测试
    long bytes = send(client_sock, reinterpret_cast<char *>(&ack), sizeof(ack_info), 0);
    if (bytes <= 0) {
        cerr << "server: send_object ack_info " << endl;
        return false;
    }
    return true;
}

// TODO 错误处理
Transmitter_server_TCP::Transmitter_server_TCP(sockaddr_in server_ad) {
    clear_member();
    server_addr = server_ad;
    addr_len = sizeof(sockaddr);

    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock == INVALID_SOCKET) {
        cerr << "server: open server socket fail" << endl;
    }

    if (bind(server_sock, reinterpret_cast<sockaddr *>(&server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        std::cerr << "server: bind address fail" << std::endl;
    }

    if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "server: cant listen request" << std::endl;
    }
}