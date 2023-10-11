#include <algorithm>
#include <array>

//my headers;
#include "Wrapped.h"


namespace Wrapped_space {
    Basic *fbs;
}

// =============================================================================
// ------------------------------- BLOCK ---------------------------------------
// =============================================================================

template<typename T>
Block<T>::Block(short id) {
    this->id = id;
    Wrapped_space::bfs->read_block(this->id, (void *) &this->raw);

}

template<typename T>
Block<T>::Block() {
    if (Wrapped_space::bfs->get_free_block() == 0) throw Wrapped_space::DiskFullException();
    this->id = Wrapped_space::bfs->get_free_block();

}

template<typename T>
T Block<T>::get_raw() {
    return this->raw;
}

template<typename T>
short Block<T>::get_id() { return this->id; }

template<typename T>
void Block<T>::write_and_set_block(T block) {
    Wrapped_space::bfs->write_block(this->id, (void *) &block);
    this->raw = block;
}


// =============================================================================
// ----------------------------- DATA BLOCK ------------------------------------
// =============================================================================
DataBlock::DataBlock(short id) : Block<Data_t>(id) {
    for (auto i = 0; i < BLOCK_SIZE; ++i)
        this->data[i] = this->raw.data[i];

}

DataBlock::DataBlock(std::array<char, BLOCK_SIZE> &data) : Block<Data_t>() {
    this->set_data(data);
}


std::array<char, BLOCK_SIZE> DataBlock::get_data() {
    return this->data;
}

void DataBlock::set_data(std::array<char, BLOCK_SIZE> data) {
    Data_t tempRaw;
    for (auto i = 0; i < data.size(); i++) tempRaw.data[i] = data[i];
    this->write_and_set_block(tempRaw);
    this->data = data;
}


// =============================================================================
// ------------------------------- INODE ---------------------------------------
// =============================================================================
template<typename T>
Inode<T>::Inode(short id) :Block<T>(id) {
    this->magic = this->raw.magic;
}

template<typename T>
Inode<T>::Inode() :Block<T>() {}

template<typename T>
unsigned int Inode<T>::get_magic() { return this->magic; }

template<typename T>
bool Inode<T>::is_dir() { return this->magic = DIR_MAGIC_NUM; }

template<typename T>
bool Inode<T>::is_file() { return this->magic = INODE_MAGIC_NUM; }

// =============================================================================
// ------------------------------- FILE INODE ----------------------------------
// =============================================================================


FileInode::FileInode(short id) : Inode(id) {
    if (!this->is_file()) {
        std::cerr << "Error (FileInode::FileInode): Inode is not a file inode." << std::endl;
        throw Wrapped_space::FileExistsException();
    }
    this->magic = this->raw.magic;
    this->size = this->raw.size;

    for (auto i = 0; i < MAX_DATA_BLOCKS; ++i) {
        short block_id = this->raw.blocks[i];
        if (block_id != UNUSED_ID) this->blocks.emplace_back(DataBlock(block_id));
    }
}

FileInode::FileInode() : Inode<Inode_t>() {
    Inode_t tempRaw;
    tempRaw.magic = INODE_MAGIC_NUM;
    tempRaw.size = 0;
    for (auto i = 0; i < MAX_DATA_BLOCKS; ++i)
        tempRaw.blocks[i] = UNUSED_ID;
    this->write_and_set_block(tempRaw);
    this->magic = tempRaw.magic;
    this->size = tempRaw.size;

}

unsigned int FileInode::get_size() {
    return this->size;
}

std::vector<DataBlock> FileInode::get_blocks() { return this->blocks; }

void FileInode::add_block(DataBlock block) {
    Inode_t tempRaw = this->get_raw();
    bool written = false;
    for (auto i = 0; i < MAX_DATA_BLOCKS; i++) {
        if (tempRaw.blocks[i] != UNUSED_ID)continue;
        tempRaw.blocks[i] = block.get_id();
        written = true;
        break;
    }
    if (!written) throw Wrapped_space::FileFullException();

    this->write_and_set_block(tempRaw);
    this->blocks.emplace_back(block);
}

void FileInode::remove_block(DataBlock block) {
    auto block_id = block.get_id();
    Inode_t tempRaw = this->get_raw();
    int index = -1;
    for (auto i = 0; i < MAX_DATA_BLOCKS; ++i) {
        if (tempRaw.blocks[i] == block_id) {
            index = 1;
            break;
        }
    }
    if (index == -1) {
        std::cerr << "ERROR (FileInode::remove_block): Block #" << block_id << " was not found in the File Inode."
                  << std::endl;
        throw Wrapped_space::FilesSystemException();
    }
    for (auto i = 0; i < MAX_DATA_BLOCKS - 1; ++i)tempRaw.blocks[i] = tempRaw.blocks[i + 1];
    tempRaw.blocks[MAX_DATA_BLOCKS - 1] = UNUSED_ID;
    this->blocks.erase(std::remove_if(this->blocks.begin(), this->blocks.end(), [&](DataBlock &block) -> bool {
        return block.get_id() == block_id;
    }), this->blocks.end());
    this->write_and_set_block(tempRaw);
}

void FileInode::set_size(unsigned int size) {
    Inode_t tempRaw = this->get_raw();
    tempRaw.size = size;
    this->write_and_set_block(tempRaw);
    this->size = tempRaw.size;
}

bool FileInode::has_free_block() {
    return MAX_DATA_BLOCKS - this->blocks.size() > 0;
}

unsigned int FileInode::internal_frag_size() {
    return this->size % BLOCK_SIZE;
}


// =============================================================================
// ------------------------------- DIRECTORY INODE -----------------------------
// =============================================================================
DirInode::DirInode(short id) : Inode(id) {
    if (!this->is_dir()) {
        std::cerr << "ERROR (DirInode::DirInode( : Inode is not a directory inode." << std::endl;
        throw Wrapped_space::FilesSystemException();
    }
    this->magic = this->raw.magic;
    this->num_entries = this->raw.num_entries;
    for (auto i = 0; i < MAX_DIR_ENTRIES; ++i) {
        char *name = this->raw.dir_entries[i].name;
        short block_id = this->raw.dir_entries[i].block_num;
        if (block_id == UNUSED_ID)continue;
        Inode inode = Inode(block_id);
        if (inode.is_file()) {
            FileInode fileInode(block_id);
            DirEntry<FileInode> entry(name, fileInode);
            this->file_entries.emplace_back(entry);
        } else if (inode.is_dir()) {
            DirInode dirInode(block_id);
            DirEntry<DirInode> entry(name, dirInode);
            this->dir_entries.emplace_back(entry);
        } else {
            std::cerr << "ERROR (DirInode::DirInode):Inode is not a file or directory inode." << std::endl;
            FileInode fileInode(block_id);
            DirEntry<FileInode> entry(name, fileInode);
            this->file_entries.emplace_back(entry);
        }
    }
}

DirInode::DirInode() : Inode() {
    Dir_t tempRaw;
    tempRaw.magic = DIR_MAGIC_NUM;
    tempRaw.num_entries = 0;
    for (auto i = 0; i < MAX_DIR_ENTRIES; ++i) tempRaw.dir_entries[i].block_num = UNUSED_ID;

    this->write_and_set_block(tempRaw);
    this->magic = tempRaw.magic;
    this->num_entries = tempRaw.num_entries;
}

unsigned int DirInode::get_num_entries() {
    return this->num_entries;

}

std::vector<DirEntry<FileInode>> DirInode::get_file_inode_entries() {
    return this->file_entries;
}

std::vector<DirEntry<DirInode>> DirInode::get_dir_inode_entries() {
    return this->dir_entries;
}

void DirInode::add_entry(DirEntry<DirInode> entry) {
    this->add_entry_base(entry, this->dir_entries);

}

void DirInode::add_entry(DirEntry<FileInode> entry) {
    this->add_entry_base(entry, this->file_entries);
}

template<typename T>
void DirInode::add_entry_base(DirEntry<T> entry, std::vector<DirEntry<T>> &vec) {
    Dir_t tempRaw = this->get_raw();
    bool written = false;
    for (auto i = 0; i < MAX_DIR_ENTRIES; ++i) {
        if (tempRaw.dir_entries[i].block_num != UNUSED_ID) continue;
        tempRaw.dir_entries[i].block_num = entry.get_inode().get_id();
        int name_size = std::min(int(entry.get_name().size()), MAX_F_NAME_SIZE);
        for (auto j = 0; j < name_size; ++j) {
            tempRaw.dir_entries[i].name[j] = entry.get_name()[j];
        }

        tempRaw.dir_entries[i].name[name_size] = '\0';
        tempRaw.num_entries++;
        written = true;
        break;
    }
    if (!written) {
        throw Wrapped_space::DirFullException();
    }
    this->write_and_set_block(tempRaw);
    this->num_entries = tempRaw.num_entries;
}

void DirInode::remove_entry(DirEntry<DirInode> entry) {
    this->remove_entry_base(entry, this->dir_entries);
}

void DirInode::remove_entry(DirEntry<FileInode> entry) {
    this->remove_entry_base(entry, this->file_entries);
}

template<typename T>
void DirInode::remove_entry_base(DirEntry<T> entry, std::vector<DirEntry<T>> &vec) {
    short block_id = entry.get_inode().get_id();
    Dir_t tempRaw = this->get_raw();

    int index = 0;
    for (auto i = 0; i < MAX_DIR_ENTRIES; ++i) {
        if (tempRaw.dir_entries[i].block_num == block_id) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        std::cerr << "ERROR (DirInode::remove_entry):DirEntry for Block #" << block_id
                  << " was not found the Dir Inode #" << this->get_id() << "." << std::endl;
        throw Wrapped_space::FilesSystemException();
    }
    for (auto i = index; i < MAX_DATA_BLOCKS; ++i) tempRaw.dir_entries[i] = tempRaw.dir_entries[i + 1];
    tempRaw.dir_entries[MAX_DIR_ENTRIES - 1].block_num = UNUSED_ID;

    tempRaw.num_entries--;
    this->num_entries = tempRaw.num_entries;
    vec.erase(std::remove_if(vec.begin(), vec.end(), [&](DirEntry<T> &curr_entry) -> bool {
        return curr_entry.get_inode().get_id() == block_id;
    }), vec.end());
    this->write_and_set_block(tempRaw);

}

bool DirInode::has_free_entry() {
    return (MAX_DIR_ENTRIES - this->get_num_entries() > 0);
}

// =============================================================================
// ------------------------------- DIR ENTRY -----------------------------------
// =============================================================================

template<typename T>
DirEntry<T>::DirEntry(std::string name, T inode) {
    this->name = name;
    this->inode_id = inode.get_id();
    if (name.size() > MAX_F_NAME_SIZE) throw Wrapped_space::FileNameTooLongException();

}

template<typename T>
std::string DirEntry<T>::get_name() {
    return this->name;
}

template<typename T>
T DirEntry<T>::get_inode() {
    return T(this->inode_id);
}



