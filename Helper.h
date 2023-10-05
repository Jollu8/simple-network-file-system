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

using namespace std::string_literals;

std::string format_response(std::string code, std::string message) {
    const std::string endline = "\r\n"s;
    std::string full_response = code + endline + "Length"s + std::to_string(message.size()) + endline + message;
    return full_response;
}