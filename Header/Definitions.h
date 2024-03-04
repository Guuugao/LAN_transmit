//
// Created by TIME LEAP MACHINE on 2023/10/8.
//
#pragma once

// some common definitions

using std::ios;
using std::ref;
using std::cout;
using std::cerr;
using std::endl;
using std::thread;
using std::string;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;

#define SERVER_PORT (9000)
#define BUFFER_SIZE (1 << 11)// 1M

// possible state
enum enum_state{
    leisure     = 0x00000000,
    sending     = 0x00000001,
    receiving   = 0x00000010,
};

// file information struct
struct File_info{
    const char* file_name;
    u_int64 file_size;
    u_int thread_amount;     // number of planned threads
};

// file data block struct
struct Block_info{
    u_int64 seek; // file data block offset
    u_int64 size; // file data block size
};


#define ACCEPT_REQUEST 1
#define REFUSE_REQUEST 0

struct Ack{
    int code;
};