#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <unistd.h>


std::string format_response(const std::string &code, const std::string &message);

void send_message(int sock_id, const std::string &message);

struct Recv_msg_t {
    std::string message;
    bool quit{};
};

Recv_msg_t recv_message_client(int sock_fd);


Recv_msg_t recv_message_server(int sock_fd);
