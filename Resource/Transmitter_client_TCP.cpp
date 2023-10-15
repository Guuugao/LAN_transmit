//
// Created by TIME LEAP MACHINE on 2023/10/13.
//
#include "Transmitter_client_TCP.h"
#include "Definitions.h"

bool is_sending_object() {
    return true;
}

int end_send_object() {
    return 0;
}

bool receive_ACK(SOCKET client_sock) {
    ACK ack = { 0 };
    ack.code = Status::accept_req;
    if (recv(client_sock, CHAR_POINTER_CAST(ack), sizeof (ACK), 0) == SOCKET_ERROR) {
        cerr << "client: receive ACK " << WSAGetLastError() << endl;
    }
    cout << "client: received ACK" << endl;
    return ack.code == Status::accept_req;
}

// TODO 由于istream不能关闭, 所以暂时写成ifstream
int start_send_object(ifstream& data_stream, sockaddr_in server_addr, string& f_name, size_t f_size) {
    if (f_name.size() > FILE_NAME_LENGTH) {
        cerr << "client: file name too long" << endl;
        return Status::error;
    }

    char buff[BUFFER_SIZE];
    File_info file_info = { 0 };

    strcpy(file_info.file_name, f_name.c_str());
    file_info.file_size = f_size;

    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_sock == INVALID_SOCKET) {
        cerr << "client: open socket " << WSAGetLastError() << endl;
        return Status::error;
    }
    cout << "client: open socket" << endl;

    if(connect(server_sock, SOCKADDR_CAST(server_addr), sizeof(sockaddr)) == SOCKET_ERROR) {
        cerr << "client: connect server " << WSAGetLastError() << endl;
        return Status::error;
    }
    cout << "client: connect server" << endl;

    if (send(server_sock, CHAR_POINTER_CAST(file_info), sizeof (File_info), 0) == SOCKET_ERROR) {
        cerr << "client: send file information " << WSAGetLastError() << endl;
        return Status::error;
    }
    cout << "client: send file information" << endl;

    if (receive_ACK(server_sock)){
        int bytes;
        while(data_stream){
            bytes = data_stream.read(buff, BUFFER_SIZE).gcount();
            if (send(server_sock, buff, bytes, 0) == SOCKET_ERROR) {
                cerr << "client: send file data " << WSAGetLastError() << endl;
                return Status::error;
            }
            cout << "client: send file data " << bytes << " bytes" << endl;
        }
    }
    closesocket(server_sock);
    cout << "client: send complete" << endl;
    return Status::complete;
}


