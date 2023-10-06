// Реализует базовую оболочку (интерфейс командной строки) для файловой системы
#pragma once

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>


struct Command {
    std::string name;
    std::string file_name;
    std::string append_data;

};

class Shell {
public:
    void unmountNFS();

    void run();

    void run_script(char *file_name);


private:

    int cs_sock;
    bool is_mounted;

    bool execute_command(std::string command_str);

    Command parse_command(std::string command_str);

    void mkdir_rpc(std::string d_name);

    void cd_rpc(std::string d_name);

    void home_rpc();

    void rmdir_rpc(std::string d_name);

    void ls_rpc();

    void create_rpc(std::string f_name);

    void append_rpc(std::string f_name, std::string data);

    void cat_rpc(std::string f_name);

    void head_rpc(std::string f_name, int n);

    void rm_rpc(std::string f_name);

    void start_rpc(std::string f_name);

    void network_command(std::string message, bool can_be_empty);

public:
    Shell() : cs_sock(-1), is_mounted(false) {}

};