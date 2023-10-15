//
// Created by TIME LEAP MACHINE on 2023/10/8.
//

#include "Transmitter_server_TCP.h"

bool is_receiving_object() {
    return true;
}

int end_receive_object() {
    return 0;
}

void wait_request(SOCKET server_sock) {
    int addr_len = sizeof(sockaddr);
    while (true) {
        sockaddr_in client_addr{};
        cout << "server: wait request" << endl;
        SOCKET client_sock = accept(server_sock, SOCKADDR_CAST(client_addr), &addr_len);
        if (client_sock == INVALID_SOCKET) {
            cerr << "server: client_sock invalid" << endl;
            continue;
        }
        cout << "server: accept request " << client_sock << endl;
        start_receive_object(client_sock);
        closesocket(client_sock);
    }
}

bool send_ACK(SOCKET client_sock) {
    ACK ack; // TODO 先默认接受, 后面再添加用户选择的功能
    ack.code = Status::accept_req;
    int bytes = send(client_sock, CHAR_POINTER_CAST(ack), sizeof(ACK), 0);
    if (bytes <= 0) {
        cerr << "server: send ACK " << WSAGetLastError() << endl;
    }
    cout << "server: send ACK" << endl;
    return ack.code == Status::accept_req;
}

int start_receive_object(SOCKET client_sock) {
    char buff[BUFFER_SIZE];
    File_info file_info = { 0 }; // 文件信息包
    int bytes; // 单次接收的字节数
    u_llong received_bytes = 0; // 记录已经接收到的文件字节数
    ofstream ofs;

    if (recv(client_sock, CHAR_POINTER_CAST(file_info), sizeof (File_info), 0) == SOCKET_ERROR) {
        cerr << "server: receive file information  " << WSAGetLastError() << endl;
        return Status::error;
    }
    cout << "server: receive file information" << endl;

    ofs.open(file_info.file_name, ios::binary | ios::out | ios::trunc);
    if (!ofs.is_open()) {
        cerr << "server: create file " << endl;
        return Status::error;
    }
    cout << "server: create file" << endl;

    if (send_ACK(client_sock)) {
        while (received_bytes < file_info.file_size) {
            bytes = recv(client_sock, buff, BUFFER_SIZE, 0);
            if (bytes == SOCKET_ERROR) {
                cerr << "server: receive file data " << WSAGetLastError() << endl;
                ofs.close(); // TODO 传输中途失败, 考虑删除未传输完毕的文件
                return Status::error;
            }
            cout << "server: receive file data " << bytes << " bytes" << endl;
            received_bytes += bytes; // 统计字节数
            ofs.write(buff, bytes); // 写入接收到的字节数
        }
    }

    ofs.close();
    cout << "server: receive complete" << endl;
    return Status::complete;
}

int server_init(const char *addr, int port) {
    // 服务器地址与端口
    sockaddr_in host_addr;
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(port);
    host_addr.sin_addr.S_un.S_addr = inet_addr(addr);

    SOCKET host_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (host_sock == INVALID_SOCKET) {
        cerr << "open server socket fail: " << WSACleanup() << endl;
        return Status::error;
    }

    if (bind(host_sock, SOCKADDR_CAST(host_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        std::cerr << "bind address fail: " << WSAGetLastError() << std::endl;
        return Status::error;
    }

    if (listen(host_sock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "cant listen request: " << WSAGetLastError() << std::endl;
        return Status::error;
    }

    // 开启子线程等待上传请求
    thread wait_req(wait_request, host_sock);
    wait_req.detach();
    return Status::complete;
}


