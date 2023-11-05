//
// Created by TIME LEAP MACHINE on 2023/10/13.
//
#include "Transmitter_client_TCP.h"
#include "Definitions.h"

bool Transmitter_client_TCP::is_sending_object() {
    return state == enum_state::sending;
}

void Transmitter_client_TCP::end_send_object() {
    if (state == enum_state::sending) state = enum_state::interrupt;
}

// TODO 可以改成传入流的方式
std::future<int> Transmitter_client_TCP::start_send_object(sockaddr_in server_ad, const char *file_path) {
    pms_client = std::promise<int>();

    if (state != enum_state::leisure) {
        pms_client.set_value(1);
        return pms_client.get_future();
    }
    else state.store(enum_state::sending); // 正在发送文件
    clear_member();

    ifs.open(file_path, std::ios::in | std::ios::binary);
    if (!ifs.good()) { // 文件打开错误
        ifs.close();
        ifs.clear();
        close(server_sock);
        pms_client.set_value(3);
        return pms_client.get_future();
    }

    req_info.file_name = get_file_name(file_path, "/\\");
    req_info.file_size = get_file_size();
    req_info.thread_amount = get_thread_amount();

    server_addr = server_ad;
    sub_thread.reserve(req_info.thread_amount);

    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock == INVALID_SOCKET) {
        std::cerr << "client: open socket " << __FUNCTION__ << std::endl;
        pms_client.set_value(2);
        return pms_client.get_future();
    }

    if (connect(server_sock, reinterpret_cast<sockaddr *>(&server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        std::cerr << "client: connect server " << __FUNCTION__ << std::endl;
        close(server_sock);
        pms_client.set_value(2);
        return pms_client.get_future();
    }

    if (send_request() && receive_ACK(server_sock) == 1) {
        close(server_sock);
        std::thread th_do_send = std::thread(&Transmitter_client_TCP::do_send, this);
        if (th_do_send.joinable()) th_do_send.detach();
        return pms_client.get_future(); // do_send函数的返回值
    }

    ifs.close();
    ifs.clear();
    state.store(enum_state::leisure); // 标识为空闲
    pms_client.set_value(5);
    return pms_client.get_future();
}

void Transmitter_client_TCP::do_send() {
    long block_size = (req_info.file_size / req_info.thread_amount) + 1;

    for (int i = 0; i < req_info.thread_amount; ++i) {
        block_info blk_info = {0};
        // 最后一个线程, 块大小不一定等于定义的块大小
        blk_info.size = (i == (req_info.thread_amount - 1)) ? (req_info.file_size % block_size) : block_size;
        blk_info.seek = block_size * i;
        sub_thread.emplace_back(std::async(&Transmitter_client_TCP::send_block,
                                           this, blk_info));
    }

    int sub_thread_rtn = 0;
    for (auto &item: sub_thread) {
        int res = item.get();
        if (res != 0) sub_thread_rtn = res;
        printf("\033[33mclient: sub thread return %d\n\033[0m", res);
    }
    state.store(enum_state::leisure); // 标识为空闲
    ifs.close();
    ifs.clear();
    pms_client.set_value(sub_thread_rtn);
}

int Transmitter_client_TCP::send_block(block_info blk_info) {
    socket_fd sub_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sub_sock == INVALID_SOCKET) {
        fprintf(stderr, "client: open sub_sock %s\n", __FUNCTION__);
        state = enum_state::error;
        return 2;
    }

    if (connect(sub_sock, reinterpret_cast<sockaddr *>(&server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        fprintf(stderr, "client: sub_thread %d connect server %s\n", sub_sock, __FUNCTION__);
        state = enum_state::error;
        close(sub_sock);
        return 2;
    }

    if (send(sub_sock, reinterpret_cast<char *>(&blk_info), sizeof(blk_info), 0) <= 0) {
        fprintf(stderr, "client: sub_thread %d send block info %s\n", sub_sock, __FUNCTION__);
        state = enum_state::error;
        close(sub_sock);
        return 2;
    }

    long send_size = 0;    // 单次应该发送的字节数
    long total_bytes = 0;  // 总计已发送字节数
    char buff[BUFFER_SIZE];
    while (total_bytes < blk_info.size) {
        printf("\033[33mclient: status %d\n\033[0m", state.load());

        // when error or interrupt, end sub thread
        if (state != enum_state::sending) {
            fprintf(stderr, "client: sub thread %d is not sending\n", sub_sock);
            close(sub_sock);
            if (state == enum_state::error) return 2;
            else if (state == enum_state::interrupt) return 4;
            else return 5;
        }

        send_size = std::min(blk_info.size - total_bytes, static_cast<long>(BUFFER_SIZE));
        {
            std::lock_guard<std::mutex> lg_ifs(m_ifs);
            printf("\033[33mclient: sub socket %d lock\n\033[0m", sub_sock);
            ifs.seekg(blk_info.seek + total_bytes);
            ifs.read(buff, send_size);
            if (send(sub_sock, buff, send_size, 0) <= 0) {
                fprintf(stderr, "client: sub_thread send block %s\n", __FUNCTION__);
                close(sub_sock);
                return 2;
            }
            total_bytes += send_size; // 更新已发送字节数
            printf("\033[33mclient:sub_thread %d send size %lu total %lu\n\033[0m", sub_sock, send_size, total_bytes);
        }
        printf("\033[33mclient: sub socket %d unlock\n\033[0m", sub_sock);
    }

    close(sub_sock);
    return 0;
}

void Transmitter_client_TCP::clear_member() {
    close(server_sock);
    server_sock = INVALID_SOCKET;
    memset(&req_info, 0, sizeof(request_info));
    memset(&server_addr, 0, sizeof(sockaddr_in));
    sub_thread.clear();
    ifs.close();
    ifs.clear();
}

int Transmitter_client_TCP::receive_ACK(socket_fd fd) {
    ack_info ack = {0};
    if (recv(fd, reinterpret_cast<char *>(&ack), sizeof(ack_info), 0) <= 0) {
        std::cerr << "client: receive ack_info " << std::endl;
        return -1;
    }
    return ack.code;
}

bool Transmitter_client_TCP::send_request() {
    return send(server_sock,
                reinterpret_cast<char *>(&req_info),
                sizeof(request_info), 0) > 0;
}

const char *Transmitter_client_TCP::get_file_name(const char *file_path, const char *separator) {
    const char *s = file_path + strlen(file_path) - 1; // 从后往前查找
    while (s != file_path) {
        const char *a = separator; // 依次比较每个分隔符
        while (*a != '\0') {
            if (*a++ == *s) return ++s;
        }
        --s;
    }
    return nullptr;
}

int Transmitter_client_TCP::get_thread_amount() {
    // 一个线程发送1G
    return static_cast<int>(req_info.file_size / (1 << 30)) + 1;
}

long Transmitter_client_TCP::get_file_size() {
    ifs.seekg(0, std::ios::end);
    long len = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    return len;
}

Transmitter_client_TCP::Transmitter_client_TCP() {
//    signal(SIGPIPE, SIG_IGN);
    state.store(enum_state::leisure);
    clear_member();
}

Transmitter_client_TCP::~Transmitter_client_TCP() {
    std::cout << "\033[33mdelete client\033[0m" << std::endl;
}