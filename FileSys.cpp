#include <cstring>
#include <iostream>
#include <unistd.h>
#include <cmath>
// my headers;

#include "FileSys.h"
#include "Basic.h"
#include "Blocks.h"
#include "Wrapped.h"
#include "Helper.h"

void validate_before_new_entry(DirInode dir, std::string name);

extern std::string format_response(std::string code, std::string message);

extern void send_message(int sock_fd, std::string message);

void FileSys::mount(int sock) {
    bfs.mount();
    curr_dir = 1;
    fs_sock = sock;

    Wrapped_space::bfs = &bfs;
    this->set_working_dir(DirInode(HOME_DIR_ID));
}

void FileSys::unmount() {
    bfs.unmount();
    close(fs_sock);
}

void FileSys::mkdir(const char *name) {
    std::string dir_name = name;
    DirInode working_dir = this->get_working_dir();
    validate_before_new_entry(working_dir, dir_name);
    DirInode new_dir = DirInode();
    DirEntry<DirInode> entry = DirEntry<DirInode>(dir_name, new_dir);
    working_dir.add_entry(entry);
    this->response_ok();

}

void FileSys::cd(const char *name) {
    std::string dir_name = name;
    DirInode working_dir = this->get_working_dir();

    for (DirEntry<DirInode> entry: working_dir.get_dir_inode_entries()) {
        if (entry.get_name() == name) {
            this->set_working_dir(entry.get_inode());
            this->response_ok();
            return;
        }
    }

    for (auto entry: working_dir.get_file_inode_entries())
        if (entry.get_name() == name)
            throw Wrapped_space::NotADirException();
    throw Wrapped_space::FileNotFoundException();


}

void FileSys::home() {
    this->set_working_dir(DirInode(HOME_DIR_ID));
    this->response_ok();
}

