// Реализует команды файловой системы, доступные оболочке.

#pragma once
// my headers;
#include <iterator>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <string>
#include <cmath>
#include "Wrapped.h"
#include "Helper.h"


class FileSys {
public:
    // mounts the file system
    void mount(int sock);

    // unmounts the file syste
    void unmount();

    // make a directory
    void mkdir(const char *name);

    // switch to a directory
    void cd(const char *name);

    // switch to home directory
    void home();

    // remove a directory
    void rmdir(const char *name);

    // list the contents of current directory
    void ls();

    // create an empty data file
    void create(const char *name);

    // append data to a data file
    void append(const char *name, const char *data);

    // display the contents of a data file
    void cat(const char *name);

    // display the first N bytes of the file
    void head(const char *name, unsigned int n);

    // delete a data file
    void rm(const char *name);

    // display stats about file or directory
    void stat(const char *name);

private:
   Basic bfs; // basic file system
    short curr_dir;   // current directory

    int fs_sock; // file server socket

    // Additional private variables and Helper functions - if desired

    void set_working_dir(DirInode dir);

    [[nodiscard]] DirInode get_working_dir() const;

    void response_ok(const std::string& message = "success") const;
};