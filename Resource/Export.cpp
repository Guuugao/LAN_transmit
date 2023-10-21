//
// Created by Cardidi on 2023/10/21.
// This is the warper for C# to call functions.
//

#include "Transmitter_server_TCP.h"
#include "Transmitter_client_TCP.h"

//
// Client Part
//

Transmitter_client_TCP* client_ptr;

extern "C++" __declspec(dllexport) bool create_client()
{
    if (client_ptr != nullptr) return false;

    client_ptr = new Transmitter_client_TCP();
    return true;
}

extern "C++" __declspec(dllexport) bool destroy_client()
{
    if (client_ptr == nullptr) return false;

    delete client_ptr;
    client_ptr = nullptr;
    return true;
}

extern "C++" __declspec(dllexport) bool client_is_sending()
{
    if (client_ptr == nullptr) return false;
    return client_ptr->is_sending_object();
}

extern "C++" __declspec(dllexport) void client_start_send()
{
    if (client_ptr == nullptr) return;
    client_ptr->end_send_object();
}

extern "C++" __declspec(dllexport) int client_interrupt_send(
        const char* file_path,
        const u_short srv_port,
        const char* srv_address)
{
    if (client_ptr == nullptr) return -1;

    sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(srv_port);
    server_addr.sin_addr.S_un.S_addr = inet_addr(srv_address);

    return client_ptr->start_send_object(server_addr, file_path);
}


//
// Server Part
//

Transmitter_server_TCP* server_ptr;

void create_server_internal(const u_short port)
{
    sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.S_un.S_addr = ADDR_ANY;

    server_ptr = new Transmitter_server_TCP(server_addr);
}


extern "C++" __declspec(dllexport) bool create_server(const u_short port)
{
    if (server_ptr != nullptr) return false;

    create_server_internal(port);
    return true;
}

extern "C++" __declspec(dllexport) bool destroy_server()
{
    if (server_ptr == nullptr) return false;

    delete server_ptr;
    server_ptr = nullptr;
    return true;
}

extern "C++" __declspec(dllexport) bool server_is_receiving()
{
    if (server_ptr == nullptr) return false;
    return server_ptr->is_receiving_object();
}

extern "C++" __declspec(dllexport) int server_start_receive()
{
    if (server_ptr == nullptr) return -1;
    return server_ptr->start_receive_object();
}

extern "C++" __declspec(dllexport) void server_interrupt_receive()
{
    if (server_ptr == nullptr) return;
    server_ptr->end_receive_object();
}