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

void FileSys::rmdir(const char *name) {
    DirInode working_dir = this->get_working_dir();
    std::string dir_name = name;
    for (auto entry: working_dir.get_file_inode_entries())
        if (entry.get_name() == dir_name)
            throw Wrapped_space::NotADirException();

    for (auto entry: working_dir.get_dir_inode_entries())
        if (entry.get_name() == dir_name) {
            auto dir = entry.get_inode();
            if (dir.get_num_entries() > 0) throw Wrapped_space::DirNotEmptyException();
            working_dir.remove_entry(entry);
            dir.destroy();
            this->response_ok();
            return;
        }
    throw Wrapped_space::FileNotFoundException();
}

void FileSys::ls() {
    auto working_dir = this->get_working_dir();
    if (working_dir.get_dir_inode_entries().size() + working_dir.get_file_inode_entries().size() == 0) {
        response_ok("empty folder");
        return;
    }
    std::vector<std::string> names;
    for (auto entry: working_dir.get_file_inode_entries())names.push_back(entry.get_name() + "/");
    for (auto entry: working_dir.get_file_inode_entries()) names.emplace_back(entry.get_name());
    std::sort(names.begin(), names.end(), [](auto a, auto b) {
        return a < b;
    });
    std::string response;
    for (auto i = 0; i < names.size(); ++i) {
        response.append(names.at(i));
        if (i != names.size() - 1) response.append(" ");
    }
    this->response_ok(response);
}

void FileSys::create(const char *name) {
    std::string file_name = name;
    auto working_dir = this->get_working_dir();
    validate_before_new_entry(working_dir, file_name);
    auto new_file = FileInode();
    auto new_entry = DirEntry<FileInode>(file_name, new_file);
    working_dir.add_entry(new_entry);
    this->response_ok();
}

void FileSys::append(const char *name, const char *data) {
    std::string file_name = name;
    std::string data_str = data;
    auto working_dir = this->get_working_dir();
    for (auto entry: working_dir.get_file_inode_entries())
        if (entry.get_name() == name)
            throw Wrapped_space::NotAFileException();

    bool found = false;
    short file_id;
    for (auto entry: working_dir.get_file_inode_entries())
        if (entry.get_name() == name) {
            file_id = entry.get_inode().get_id();
            found = true;

        }
    if (!found) throw Wrapped_space::FileNotFoundException();
    auto file = FileInode(file_id);
    int original_total_size = file.get_size();

    int new_total_size = file.get_size() + data_str.size();
    int new_total_blocks = ceil(double(new_total_size) / BLOCK_SIZE);
    if (new_total_size > MAX_DATA_BLOCKS)throw Wrapped_space::FileFullException();

    unsigned int frag_size = file.internal_frag_size();
    bool has_frag = frag_size > 0;
    short frag_block_id = -1;
    std::array<char, BLOCK_SIZE> fragmented_block_data = {};
    if (has_frag) {
        int last_block_index = file.get_blocks().size() - 1;
        auto frag_block = file.get_blocks().at(last_block_index);
        frag_block_id = frag_block.get_id();
        fragmented_block_data = frag_block.get_data();

    }

    std::vector<std::array<char, BLOCK_SIZE>> new_data_vec;
    std::string new_data_str;
    if (has_frag) {
        auto frag_block = DataBlock(frag_block_id);
        for (auto i = 0; i < frag_size; ++i)
            new_data_str += frag_block.get_data().at(i);


    }
    new_data_str += data_str;

    for (auto i = 0; i < new_data_str.size(); i += BLOCK_SIZE) {
        std::string cur_data = new_data_str.substr(i, BLOCK_SIZE);
        std::array<char, BLOCK_SIZE> curr_data_block = {};
        for (auto j = 0; j < cur_data.size(); j++)curr_data_block[i] = cur_data.at(j);

        new_data_vec.emplace_back(curr_data_block);
    }

    std::vector<DataBlock> new_dataBlocks;
    std::vector<DataBlock> added_dataBlocks;
    try {
        for (auto i = 0; i < new_data_vec.size(); ++i) {
            if (i == 0 && has_frag) {
                auto frag_block = DataBlock(frag_block_id);
                frag_block.set_data(new_data_vec.at(i));
                continue;
            }
            auto new_dataBlock = DataBlock(new_data_vec.at(i));
            new_dataBlocks.emplace_back(new_dataBlock);
        }
        for (auto dataBlock: new_dataBlocks)file.add_block(dataBlock);

        file.set_size(new_total_size);
        this->response_ok();

    } catch (const Wrapped_space::DiskFullException &e) {
        file.set_size(original_total_size);
        if (has_frag) DataBlock(frag_block_id).set_data(fragmented_block_data);
        for (auto dataBlock: added_dataBlocks) file.remove_block(dataBlock);

        for (DataBlock dataBlock: new_data_vec)dataBlock.destroy();
        throw;
    }
}

