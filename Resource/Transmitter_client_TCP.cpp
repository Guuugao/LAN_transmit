//
// Created by TIME LEAP MACHINE on 2023/10/13.
//
#include "Transmitter_client_TCP.h"
#include "Definitions.h"

bool Transmitter_client_TCP::is_sending_object() {
    return state == enum_state::sending;
}

void Transmitter_client_TCP::end_send_object() {
    state = enum_state::interrupt;
}

// TODO 可以改成传入流的方式
int Transmitter_client_TCP::start_send_object(sockaddr_in server_ad, const char *file_path) {
    if (state != enum_state::leisure) return 1;
    else state.store(enum_state::sending); // 正在发送文件
    clear_member();

    ifs.open(file_path, ios::in | ios::binary);
    if (!ifs.good()) { // 文件打开错误
        ifs.close(); ifs.clear();
        close(server_sock);
        return 3;
    }

    req_info.file_name = get_file_name(file_path, "/\\");
    req_info.file_size = get_file_size();
    req_info.thread_amount = get_thread_amount();

    server_addr = server_ad;
    long block_size = (req_info.file_size / req_info.thread_amount) + 1;
    sub_thread.reserve(req_info.thread_amount);

    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_sock == INVALID_SOCKET) {
        cerr << "client: open socket " << endl;
        return 2;
    }

    if(connect(server_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        cerr << "client: connect server " << endl;
        close(server_sock);
        return 2;
    }

    int sub_thread_rtn = 0; // 子线程返回的状态码
    if (send_request() && receive_ACK()){

        printf("client: file path %s\n",file_path);

        close(server_sock);
        for (int i = 0; i < req_info.thread_amount; ++i) {
            block_info block_info = { 0 };
            // 最后一个线程, 块大小不一定等于定义的块大小
            block_info.size = (i == (req_info.thread_amount - 1)) ? (req_info.file_size % block_size) : block_size;
            block_info.seek = block_size * i;
            sub_thread.emplace_back(std::async(&Transmitter_client_TCP::send_block, this,
                                                block_info));
        }

        for (auto &item: sub_thread){
            int res = item.get();
            if (res != 0) sub_thread_rtn = res;
        }
    }
    if (sub_thread_rtn == 0) cout << "client: send_object complete" << endl;
    ifs.close();ifs.clear();
    state.store(enum_state::leisure); // 标识为空闲
    return sub_thread_rtn;
}

int Transmitter_client_TCP::send_block(block_info block_info) {
    socket_fd sub_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sub_sock == INVALID_SOCKET) {
        printf("ERR client: open sub_sock\n");
        state = enum_state::error;
        return 2;
    }

    long send_size = 0;    // 单次应该发送的字节数
    long total_bytes = 0;  // 总计已发送字节数
    char buff[BUFFER_SIZE];

    if(connect(sub_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        printf("ERR client: sub_thread connect server\n");
        state = enum_state::error;
        close(sub_sock);
        return 2;
    }

    if (send(sub_sock, reinterpret_cast<char *>(&block_info), sizeof(block_info), 0) <= 0){
        printf("ERR client: sub_thread send_object block info\n");
        state = enum_state::error;
        close(sub_sock);
        return 2;
    }

    while (total_bytes < block_info.size) {
        std::lock_guard<std::mutex> lg_m_ifs(m_ifs);

        // when error or interrupt, end sub thread
        if (state != enum_state::sending){
            close(sub_sock);
            if (state == enum_state::error) return 2;
            else if (state == enum_state::interrupt) return 4;
        }

        send_size = std::min(block_info.size - total_bytes, static_cast<long>(BUFFER_SIZE));

        ifs.seekg(block_info.seek + total_bytes);
        ifs.read(buff, send_size);
        if (send(sub_sock, buff, send_size, 0) <= 0) {
            cout << "ERR client: sub_thread send_object block" << endl;
            close(sub_sock);
            return 2;
        }
        total_bytes += send_size; // 更新已发送字节数
    }

    close(sub_sock);
    return 0;
}

void Transmitter_client_TCP::clear_member() {
    close(server_sock);
    server_sock = INVALID_SOCKET;
    memset(&req_info, 0, sizeof (request_info));
    memset(&server_addr, 0, sizeof (sockaddr_in));
    sub_thread.clear();
    ifs.close(); ifs.clear();
}

bool Transmitter_client_TCP::receive_ACK() {
    ack_info ack = { 0 };
    if (recv(server_sock, reinterpret_cast<char*>(&ack), sizeof (ack_info), 0) <= 0) {
        cerr << "client: receive ack_info " << endl;
        return false;
    }
    return ack.code == 1;
}

bool Transmitter_client_TCP::send_request(){
    return send(server_sock, reinterpret_cast<char *>(&req_info), sizeof(request_info), 0) > 0;
}

const char *Transmitter_client_TCP::get_file_name(const char* file_path, const char* separator) {
    const char* s = file_path + strlen(file_path) - 1; // 从后往前查找
    while (s != file_path){
        const char *a = separator; // 依次比较每个分隔符
        while (*a != '\0'){
            if (*a++ == *s) return ++s;
        }
        --s;
    }
    return nullptr;
}

int Transmitter_client_TCP::get_thread_amount(){
    if (req_info.file_size <= (1ULL << 30)) { // 文件大小 <= 1G
        return 1;
    } else if (req_info.file_size <= (1ULL << 32)) { // 文件大小 <= 4G
        return 2;
    } else {
        return 4;
    }
}

long Transmitter_client_TCP::get_file_size() {
    ifs.seekg(0, ios::end);
    long len = ifs.tellg();
    ifs.seekg(0, ios::beg);

    printf("client: file size %ld\n", len);

    return len;
}

Transmitter_client_TCP::Transmitter_client_TCP() {
    state = enum_state::leisure;
    clear_member();
}