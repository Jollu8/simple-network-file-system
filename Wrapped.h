#pragma once

#include <unistd.h>
#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <exception>

//my headers;
#include "Basic.h"
#include "Blocks.h"

const short UNUSED_ID = 0;
const short HOME_DIR_ID = 1;
namespace Wrapped_space {
    extern Basic *bfs;

    class FilesSystemException : public std::exception {
    public:
        const char *what() const throw() {
            return "A File system error has occurred!";
        }
    };

    class NotADirException : public FilesSystemException {
    public:
        const char *what() const throw() {
            return "500 File is not a directory";

        }
    };

    class FileExistsException : public FilesSystemException {
    public:
        const char *what() const throw() {
            return "502 File exist";
        }
    };

    class FileNotFoundException : public FilesSystemException {
    public:
        const char *what() const throw() {
            return "503 File does not exist";

        }
    };

    class FileNameTooLongException : public FilesSystemException {
    public:
        const char *what() const throw() {
            return "504 File name is too long";
        }
    };

    class DiskFullException : public FilesSystemException {
    public:
        const char *what() const throw() {
            return "505 Disk is full";
        }
    };

    class DirFullException : public FilesSystemException {
    public:
        const char *what() const throw() {
            return "506 Directory is full";
        }
    };

    class DirNotEmptyException : public FilesSystemException {
    public:
        const char *what() const throw() {
            return "507 Directory is not empty";
        }
    };

    class FileFullException : public FilesSystemException {
    public:
        const char *what() const throw() {
            return "508 Append exceeds maximum file size";
        }

    };
}

// =============================================================================
// -------------------------------- BLOCK --------------------------------------
// =============================================================================
template<typename T>
class Block {
    // `Block` - это достаточно абстрактный класс, предназначенный для использования его подклассами для
// чтения и записи "сырых" структур с диска и на диск. Он представляет собой класс
// шаблон, позволяющий использовать различные типы "сырых" структур (например, `inode_t`).
public:
    Block(short id);

    Block();

    T get_raw();

    short get_id();

    void destroy() {
        Wrapped_space::bfs->reclaim_block(this->get_id());
    }

protected:
    short id;
    T raw;

    void write_and_set_block(T block);
};

// =============================================================================
// ----------------------------- DATA BLOCK ------------------------------------
// =============================================================================
class DataBlock : public Block<Data_t> {
public:
    DataBlock(short id);

    DataBlock(std::array<char, BLOCK_SIZE> &data);

    std::array<char, BLOCK_SIZE> get_data();

    void set_data(std::array<char, BLOCK_SIZE> data);

    using Block<Data_t>::get_raw;
    using Block<Data_t>::get_id;
    using Block<Data_t>::destroy;
protected:
    // Блок использует массив, а не вектор, поскольку нет возможности определить
    // размер данных из структуры data_t (некоторые биты блока данных могут быть
    // неиспользуемыми).
    // Мои благодарности Константину Владимирову на этот совет
    std::array<char, BLOCK_SIZE> data;
};


// =============================================================================
// ------------------------------- INODE ---------------------------------------
// =============================================================================

template<typename T>
class Inode : protected Block<T> {
public:
    Inode(short id);

    Inode();

    unsigned int get_magic();

    bool is_dir();

    bool is_file();

    using Block<T>::get_raw;
    using Block<T>::get_id;
    using Block<T>::destroy;
protected:
    unsigned int magic;

};

// =============================================================================
// ----------------------------- FILE INODE ------------------------------------
// =============================================================================
class FileInode : public Inode<Inode_t> {
public:
    FileInode(short id);

    FileInode();

    unsigned int get_size();

    std::vector<DataBlock> get_blocks();

    void add_block(DataBlock block);

    void remove_block(DataBlock block);

    void set_size(unsigned int size);

    bool has_free_block();

    unsigned int internal_frag_size();

protected:
    unsigned int size;
    std::vector<DataBlock> blocks;

};


// =============================================================================
// ----------------------------- DIRECTORY INODE -------------------------------
// =============================================================================
// Форвардное объявление для `DirEntry`

template<typename T>
class DirEntry;

class DirInode : public Inode<Dir_t> {
public:
    DirInode(short id);

    DirInode();

    unsigned int get_num_entries();

    std::vector<DirEntry<FileInode>> get_file_inode_entries();

    std::vector<DirEntry<DirInode>> get_dir_inode_entries();

    void add_entry(DirEntry<FileInode> entry);

    void add_entry(DirEntry<DirInode> entry);

    // `remove_entry` удалит запись только из Dir Inode. При этом не происходит
    // уничтожить блоки, связанные с этим Inode. Вы должны сделать это вручную!

    void remove_entry(DirEntry<FileInode> entry);

    void remove_entry(DirEntry<DirInode> entry);

    bool has_free_entry();

protected:
    // Файловые и Dir-Inode хранятся отдельно, поскольку векторы могут содержать только // один тип (даже если они имеют общий базовый класс).
    // один тип (даже если они имеют общий базовый класс). Это означает, что существуют
    // также разные функции для получения и добавления инодов.
    std::vector<DirEntry<FileInode>> file_entries;
    std::vector<DirEntry<DirInode>> dir_entries;
    unsigned int num_entries;

private:
    template<class T>
    void add_entry_base(DirEntry<T> entry, std::vector<DirEntry<T>> &vec);

    template<class T>
    void remove_entry_base(DirEntry<T> entry, std::vector<DirEntry<T>> &vec);
};



// =====================
// ----- DIR ENTRY -----
// =====================

// Записи Dir хранятся внутри инода каталога и служат указателями на
// другие иноды (файловые или каталожные). Для того чтобы использовать этот класс `DirEntry`
// необходимо указать тип инода, на который указывает запись.
//
// Пример использования:
// DirInode home = DirInode(1);
// FileInode file = FileInode(2);
// DirEntry<FileInode> file_entry(home, file);

template<typename T>
class DirEntry {
    // static_assert(is_base_of<Inode<T>, T>::value, "DirEntry must be used with an Inode");
public:
    DirEntry(std::string name, T inode);

    std::string get_name();

    T get_inode();

protected:
    std::string name;
    short inode_id;

};

