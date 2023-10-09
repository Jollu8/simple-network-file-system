#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Shell.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

//my headers;
#include "Helper.h"

static const std::string PROMPT_STRING = "NFS";
const std::string endline = "\n";


// Смонтировать сетевую файловую систему с именем сервера и номером порта в формате server:port
void Shell::mountNFS(std::string fs_loc) {
    // создать сокет cs_sock и подключить его к серверу и порту, указанному в fs_loc
    // если все вышеперечисленные операции выполнены успешно, установите значение is_mounted в true

    // создаем вектор с местоположением filesys, именем сервера и портом
    // vector<string> filesys_addr;

    std::string hostname, port, junk;

    size_t curr = 0;
    size_t delimit_idx;
    std::istringstream ss(fs_loc);
    std::string token;
    if(std::getline(ss, token, ':')) {
        hostname = token;
        if(getline(ss, token))port = token;
    }
    addrinfo *addr, hints;
    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = PF_UNSPEC;

    int ret;
    if((ret = getaddrinfo(hostname.c_str(), port.c_str(), &hints , &addr)) != 0) {
        std::cout << "Error: Could not obtain address information for << \"" << hostname << ":" << port << "\"" << std::endl;
        std::cout << "\tgetaddrinfo returned with " << ret << std::endl;
        exit(1);
    }
    if(cs_sock < 0) {
        std::cout << "Errr: Failed to create a socket" << std::endl;
        std::cout << "\tsocket returned with " << cs_sock << std::endl;
        exit(1);
    }
    int connect_ret = connect(cs_sock, addr->ai_addr, addr->ai_addrlen);
    if(connect_ret < 0 ){
        std::cout << "Error: Failed to connect with server (" << hostname << ":" << port << ")." << std::endl;
        std::cout << "tDouble check that the server is running." << std::endl;
        exit(1);
    }
    freeaddrinfo(addr);

    is_mounted = true;
}

void Shell::unmountNFS() {
    if(is_mounted){
        close(cs_sock);
        is_mounted = false;
    }

}

void Shell::mkdir_rpc(std::string d_name) {
    std::string cmd = "mkdir " +d_name + endline;
    network_command(cmd, false);
}

void Shell::cd_rpc(std::string d_name) {
    std::string cmd = "cd " + d_name + endline;
    network_command(cmd, false);
}

void Shell::home_rpc() {
    std::string cmd = "home" + endline;
    network_command(cmd, false);
}

void Shell::rmdir_rpc(std::string d_name) {
    std::string cmd = "rmdir " + d_name + endline;
    network_command(cmd, false);

}

void Shell::ls_rpc() {
    std::string cmd = "ls " + endline;
    network_command(cmd, false);
}

void Shell::create_rpc(std::string f_name) {
    std::string cmd = "create " + f_name + endline;
    network_command(cmd, false);
}
void Shell::append_rpc(std::string f_name, std::string data) {
    std::string cmd = "append " + f_name + " " +data + endline;
    network_command(cmd, false);
}

void Shell::cat_rpc(std::string f_name) {
    std::string cmd = "cat " + f_name + endline;
    network_command(cmd, true);
}

void Shell::head_rpc(std::string f_name, int n) {
    std::string cmd = "head " + f_name + " " + std::to_string(n) + endline;
    network_command(cmd, true);
}

void Shell::rm_rpc(std::string f_name) {
    std::string cmd = "rm " + f_name + endline;
    network_command(cmd, false);
}

void Shell::start_rpc(std::string f_name) {
    std::string cmd = "stat " + f_name + endline;
    network_command(cmd, false);

}
