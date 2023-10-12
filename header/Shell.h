// Реализует базовую оболочку (интерфейс командной строки) для файловой системы
#pragma once

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <unordered_map>

//my headers;
#include "Helper.h"

enum ENUM_COMMANDS {
    E_EMPTY , E_MKDIR, E_CD, E_HOME, E_RMDIR, E_LS, E_CREATE, E_APPEND, E_CAT, E_HEAD, E_RM,
    E_STAT, E_QUIT
};
struct Command {
    std::string name;
    std::string file_name;
    std::string append_data;

};

class Shell {
public:
    void mountNFS(const std::string& fs_loc);
    void unmountNFS();

    void run();

    void run_script(char *file_name);


private:

    int cs_sock;
    bool is_mounted;

    bool execute_command(const std::string& command_str);

    static  Command parse_command(const std::string& command_str);

    void mkdir_rpc(const std::string& d_name);

    void cd_rpc(const std::string& d_name);

    void home_rpc();

    void rmdir_rpc(const std::string& d_name);

    void ls_rpc();

    void create_rpc(const std::string& f_name);

    void append_rpc(std::string f_name, const std::string& data);

    void cat_rpc(const std::string& f_name);

    void head_rpc(const std::string& f_name, int n);

    void rm_rpc(const std::string& f_name);

    void stat_rpc(const std::string& f_name);

    void network_command(const std::string& message, bool can_be_empty) const;

public:
    Shell() : cs_sock(-1), is_mounted(false) {}

};