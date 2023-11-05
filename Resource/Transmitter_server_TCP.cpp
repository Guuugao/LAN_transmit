//
// Created by TIME LEAP MACHINE on 2023/10/8.
//

#include "Transmitter_server_TCP.h"

bool Transmitter_server_TCP::is_receiving_object() {
    return state == enum_state::receiving;
}

void Transmitter_server_TCP::end_receive_object() {
    if (state == enum_state::receiving) state = enum_state::interrupt;
}

std::future<int> Transmitter_server_TCP::start_receive_object(const char *save_path) {
    pms_server = std::promise<int>();

    if (state != enum_state::leisure) {
        pms_server.set_value(1);
        return pms_server.get_future();
    }
    state.store(enum_state::receiving);
    clear_member();

    sockaddr_in client_addr = {0};
    socket_fd client_sock = accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);

    if (recv(client_sock, reinterpret_cast<char *>(&req_info), sizeof(request_info), 0) <= 0) {
        std::cerr << "server: receive file information " << __FUNCTION__ << std::endl;
        pms_server.set_value(2);
        return pms_server.get_future();
    }

    std::cout << "\033[32m" << "server: request info"
              << " " << req_info.file_name
              << " " << req_info.file_size
              << " " << req_info.thread_amount << "\033[0m" << std::endl;

    // 拼接存储路径与文件名
    char *save_path_copy = new char[strlen(save_path) + 128];
    strcpy(save_path_copy, save_path);
    save_path_copy = strcat(save_path_copy, req_info.file_name);

    sub_thread.reserve(req_info.thread_amount);
    ofs.open(save_path_copy, std::ios::out | std::ios::binary);
    delete[] save_path_copy;
    if (!ofs.good()) {
        std::cerr << "server: create file " << __FUNCTION__ << std::endl;
        ofs.close();
        ofs.clear();
        close(client_sock);
        pms_server.set_value(3);
        return pms_server.get_future();
    }

    if (!send_ACK(client_sock, 1)) {
        std::cerr << "server: send ACK(1) " << __FUNCTION__ << std::endl;
        close(client_sock);
        ofs.close();
        ofs.clear();
        state.store(enum_state::leisure);
        pms_server.set_value(2);
        return pms_server.get_future();
    } else {
        close(client_sock);
        std::thread th_do_receive = std::thread(&Transmitter_server_TCP::do_receive, this);
        // TODO double free or corruption (out), 需要join才不会出错
        if (th_do_receive.joinable()) th_do_receive.detach();
        return pms_server.get_future();
    }
}

void Transmitter_server_TCP::do_receive() {
    for (int i = 0; i < req_info.thread_amount; ++i) {
        sub_thread.emplace_back(std::async(&Transmitter_server_TCP::receive_block, this));
    }

    int sub_thread_rtn = 0;
    for (auto &item: sub_thread) {
        int res = item.get();
        if (res != 0) sub_thread_rtn = res;
        printf("\033[32mserver: sub thread return %d\n\033[0m", res);
    }

    state.store(enum_state::leisure);
    ofs.close();
    ofs.clear();
    // 错误记录: 下面这行代码(pms_server.set_value)原本位于state.store之前
    // 而该线程是detach类型, 当运行到pms_server.set_value之后
    // 外部会立刻获取值, 紧接着服务器对象析构
    // 进而导致在对象析构之后, state.store以及之后两句才开始执行
    // 而此时对应的内存已经释放, 所以报错"double free or corruption (out)"
    pms_server.set_value(sub_thread_rtn);
}

int Transmitter_server_TCP::receive_block() {
    sockaddr_in client_addr = { 0 };
    socket_fd sub_sock = accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);

    if (sub_sock == INVALID_SOCKET) {
        fprintf(stderr, "server: sub_thread open socket %s\n", __FUNCTION__);
        state = enum_state::error;
        return 2;
    }

    block_info blk_info = { 0 };
    if (recv(sub_sock, reinterpret_cast<char *>(&blk_info), sizeof(block_info), 0) <= 0) {
        fprintf(stderr, "server: sub_thread %d receive block info %s\n", sub_sock, __FUNCTION__);
        state = enum_state::error;
        close(sub_sock);
        return 2;
    }

    printf("\033[32mserver: sub thread %d receive block info: %ld %ld\n\033[0m",
           sub_sock, blk_info.seek, blk_info.size);

    long bytes = 0;            // 单次接收的字节数
    long total_bytes = 0;      // 记录已经接收到的文件字节数
    char buff[BUFFER_SIZE];
    while (total_bytes < blk_info.size) {
        printf("\033[32mserver: status %d\n\033[0m", state.load());

        if (state != enum_state::receiving) {
            fprintf(stderr, "server: sub thread %d is not receiving\n", sub_sock);
            close(sub_sock);
            if (state == enum_state::error) return 2;
            else if (state == enum_state::interrupt) return 4;
            else return 5;
        }

        bytes = recv(sub_sock, buff, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            fprintf(stderr, "server: sub_thread receive block %s\n", __FUNCTION__);
            state = enum_state::error;
            close(sub_sock);
            return 2;
        }

        {
            std::lock_guard<std::mutex> lg_ofs(m_ofs);
            printf("\033[32mserver: sub socket %d lock\n\033[0m", sub_sock);
            ofs.seekp(blk_info.seek + total_bytes); // 设置数据填充位置
            ofs.write(buff, bytes);
            total_bytes += bytes;
            printf("\033[32mserver: sub socket %d received %lu total %lu\n\033[0m", sub_sock, bytes, total_bytes);
        }
        printf("\033[32mserver: sub socket %d unlock\n\033[0m", sub_sock);
    }

    close(sub_sock);
    return 0;
}

void Transmitter_server_TCP::clear_member() {
    memset(&req_info, 0, sizeof(request_info));
    memset(&server_addr, 0, sizeof(sockaddr_in));
    sub_thread.clear();
    ofs.close();
    ofs.clear();
}

bool Transmitter_server_TCP::send_ACK(socket_fd fd, int code) {
    ack_info ack = {0};
    ack.code = code;
    long bytes = send(fd, reinterpret_cast<char *>(&ack), sizeof(ack_info), 0);
    if (bytes <= 0) {
        std::cerr << "server: send_object ack_info " << std::endl;
        return false;
    }
    return true;
}

// TODO 错误处理
Transmitter_server_TCP::Transmitter_server_TCP(sockaddr_in server_ad) {
    state.store(enum_state::leisure);
    clear_member();
    server_addr = server_ad;
    addr_len = sizeof(sockaddr);

    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock == INVALID_SOCKET) {
        std::cerr << "server: open server socket fail" << std::endl;
        state.store(enum_state::error);
    }

    if (bind(server_sock, reinterpret_cast<sockaddr *>(&server_addr),
             sizeof(sockaddr)) == SOCKET_ERROR) {
        std::cerr << "server: bind address fail" << std::endl;
        state.store(enum_state::error);
    }

    if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "server: cant listen request" << std::endl;
        state.store(enum_state::error);
    }
}

Transmitter_server_TCP::~Transmitter_server_TCP() {
    close(server_sock);
    std::cout << "\033[32mdelete server\033[0m" << std::endl;
}
