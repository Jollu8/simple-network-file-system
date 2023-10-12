#include <iostream>
#include <string>

#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unordered_map>


//my headers;

#include "header/FileSys.h"
#include "header/Helper.h"


const int BACKBLOG = 5; // // Молчаливое максимальное значение, определенное C.

enum CommandType {
    mkdir,
    ls,
    cd,
    home,
    rmdir_cmd, // Prevent name colision with an existing `rmdir` function.
    create,
    append,
    stat,
    cat,
    head,
    rm,
    quit,
    noop,
    invalid

};
std::unordered_map<std::string, CommandType> my_Commands_map{
        {"home",   home},
        {"mkdir",  mkdir},
        {"ls",     ls},
        {"cd",     cd},
        {"rmdir",  rmdir_cmd},
        {"create", create},
        {"append", append},
        {"stat",   stat},
        {"cat",    cat},
        {"head",   head},
        {"rm",     rm}
};


struct Command_ {
    CommandType type;
    std::string file;
    std::string data;
};

sockaddr_in get_server_addr(in_port_t port);

Command_ parse_command(const std::string &message);

void exec_command(int sock_fd, FileSys &fs, const Command_& command);

void response_error(std::string message);










int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./nfsserver port#\n";
        return -1;
    }
    int port = atoi(argv[1]);

    int sockfd = socket(PF_INET, SOCK_STREAM, PF_UNSPEC);

    if (sockfd < 0) {
        std::cerr << "Error: Failed tot create socket" << std::endl;
        exit(1);
    }
    int opts = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));

    // BIND: привязка к заданному номеру порта
    sockaddr_in server_addr = get_server_addr(port);
    if (bind(sockfd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Failed to bind to port " << port << std::endl;
        exit(1);
    }
    if (listen(sockfd, BACKBLOG) < 0) {
        std::cerr << "Error: Failed to listen for connections" << std::endl;
        exit(1);
    }
    std::cout << "Server is ready to handle connections" << std::endl;

    while (true) {
        try {
            sockaddr_in client_addr{};
            int client_len;
            int new_sockfd = accept(sockfd, (sockaddr *) &client_addr, (socklen_t *) &client_len);
            if (new_sockfd < 0) {
                std::cerr << "Error: Failed to accept socket connection" << std::endl;
                break;
            }
            FileSys fs{};
            fs.mount(new_sockfd);
            Recv_msg_t msg;
            msg.quit = false;
            while (!msg.quit) {
                try {
                    msg = recv_message_server(new_sockfd);
                    if (msg.quit) break;
                    std::string message = msg.message;
                    Command_ command = parse_command(message);
                    if (command.type == invalid) send_message(new_sockfd, format_response(command.data, command.data));
                    else if (command.type == noop) send_message(new_sockfd, format_response("200 OK", ""));
                    else exec_command(new_sockfd, fs, command);


                } catch (...) {
                    send_message(new_sockfd, format_response("Command Failed,", "Command Failed"));
                }
            }
            close(new_sockfd);
            fs.unmount();

        } catch (...) {
            // Это соединение не удалось. Игнорируйте неудачу, чтобы разрешить дальнейшее подключение

        }
        close(sockfd);
    }
}

sockaddr_in get_server_addr(in_port_t port) {
    sockaddr_in server_addr{};
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return server_addr;

}


void exec_command(int sock_fd, FileSys &fs, const Command_& command) {
    CommandType type = command.type;
    const char *file = command.file.c_str();
    const char *data = command.file.c_str();
    try {
        switch (type) {
            case mkdir:
                fs.mkdir(file);
                break;

            case ls:
                fs.ls();
                break;

            case cd:
                fs.cd(file);
                break;
            case home:
                fs.home();
                break;
            case rmdir_cmd:
                fs.rmdir(file);
                break;
            case create:
                fs.create(file);
                break;
            case append:
                fs.append(file, data);
                break;
            case stat:
                fs.stat(file);
                break;
            case cat:
                fs.cat(file);
                break;
            case head:
                fs.head(file, std::stoi(data));
                break;
            case rm:
                fs.rm(file);
                break;
            case quit:
                break;
            case noop:
                break;
            case invalid:
                break;
        }
    }
    catch (const Wrapped_space::FilesSystemException &e) {
        std::string err_msg = e.what();
        std::string formatted_message = format_response(err_msg, "");
        send_message(sock_fd, formatted_message);
    }
}

Command_ parse_command(const std::string &message) {
    Command_ cmd;
    std::istringstream ss(message);
    std::string name;
    int token = 0;
    if (ss >> name) {
        ++token;
        if (ss >> cmd.file) ++token;
        if (ss >> cmd.data)++token;
        std::string garbage;
        if (ss >> garbage) ++token;
    }
    switch (my_Commands_map.find(name)->second) {
        case mkdir:
            cmd.type = mkdir;
            break;
        case ls:
            cmd.type = ls;
            break;
        case cd:
            cmd.type = cd;
            break;
        case home:
            cmd.type = home;
            break;
        case rmdir_cmd:
            cmd.type = rmdir_cmd;
            break;
        case create:
            cmd.type = create;
            break;
        case append:
            cmd.type = append;
            break;
        case stat:
            cmd.type = stat;
            break;
        case cat:
            cmd.type = cat;
            break;
        case head:
            cmd.type = head;
            break;
        case rm:
            cmd.type = rm;
            break;
        default: {
            if (token == 0)cmd.type = noop;
            else {
                cmd.type = invalid;
                cmd.data = "Invalid command: " + message;
            }
            break;
        }
    }

    CommandType type = cmd.type;
    if (type == ls || type == home || type == quit) {
        if (token != 1) {
            cmd.type = invalid;
            cmd.data = "Invalid command: not enough arguments. Requires 1 token";
        }
    } else if (type == mkdir ||
               type == cd ||
               type == rmdir_cmd ||
               type == create ||
               type == cat ||
               type == rm ||
               type == stat) {
        if (token != 2) {
            cmd.type = invalid;
            cmd.data = "Invalid command: not enough arguments. Requires 2 tokens";
        }
    } else if (type == append || type == head)
        if (token != 3) {
            cmd.type = invalid;
            cmd.data = "Invalid command: not enough arguments. Requires 3 tokens";
        }
    return cmd;
}



