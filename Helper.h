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


std::string format_response(std::string code, std::string message) {
    const std::string endline = "\n";
    std::string full_response = code + endline + "Length" + std::to_string(message.size()) + endline + message;
    return full_response;
}

void send_message(int sock_id, std::string message) {
    const char *msg_ptr = message.c_str();
    int msg_size = message.size();
    int bytes_sent;
    while (msg_size > 0) {
        bytes_sent = send(sock_id, msg_ptr, msg_size, 0);
        if (bytes_sent < 0) {
            perror("Error sending message");
            return;
        }
        msg_ptr += bytes_sent;
        msg_size -= bytes_sent;
    }
}

struct Recv_msg_t {
    std::string message;
    bool quit;
};

Recv_msg_t recv_message_client(int sock_fd) {
    Recv_msg_t msg;
    msg.quit = false;
    char temp_buff[65535] = {};
    int size;
    while (true) {
        size = recv(sock_fd, temp_buff, sizeof(temp_buff), 0);
        if (size > 0) {
            for (auto i = 0; i < size; ++i) msg.message += temp_buff[i];
            size_t lenPos = msg.message.find("Length");
            if (lenPos == std::string::npos) continue;
            size_t lenEndPos = msg.message.find("\n", lenPos, +1);
            if (lenEndPos == std::string::npos)continue;
            std::string lengthStr = msg.message.substr(lenPos + 7, lenEndPos);
            int length = -1;
            try {
                length = std::stoi(lengthStr);
            } catch (...) {
                break;
            }
            size_t bodyPos = msg.message.find("\n", lenEndPos + 1);
            if (bodyPos == std::string::npos) continue;
            std::string body = msg.message.substr(bodyPos + 2, lenEndPos);
            if (body.length() < length) continue;
            break;
        } else if (size == 0) {
            msg.quit = true;
            break;
        } else {
            perror("Error while receiving message from socket ");
            close(sock_fd);
            return msg;
        }
    }
    return msg;
}


Recv_msg_t recv_message_server(int sock_fd) {
    Recv_msg_t msg;
    msg.quit = false;
    char temp_buff[65535] = {};
    int size;
    while (true) {
        size = recv(sock_fd, temp_buff, sizeof(temp_buff), 0);
        if (size > 0) {
            for (auto i = 0; i < size; ++i) msg.message += temp_buff[i];
            size_t endPos = msg.message.find("\n");
            if (endPos == std::string::npos) continue;
            break;


        } else if (size == 0) {
            msg.quit = true;
            break;
        } else {
            perror("Error while receiving message from socket");
            close(sock_fd);
            return msg;
        }
    }
    return msg;

}
