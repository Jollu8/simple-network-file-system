// Блоки, составляющие файловую систему

#pragma once

const int BLOCK_SIZE = 128;
const int NUM_BLOCKS = (BLOCK_SIZE * 8);
const int MAX_F_NAME_SIZE = 9;
const int MAX_DIR_ENTRIES = ((BLOCK_SIZE - 8) / 12);

const int MAX_DATA_BLOCKS = ((BLOCK_SIZE - 8) - 2);
const int MAX_FILE_SIZE = (MAX_DATA_BLOCKS * BLOCK_SIZE);


// magic numbers
const unsigned DIR_MAGIC_NUM = 0XFFFFFFFF;
const unsigned INODE_MAGIC_NUM = 0xFFFFFFFE;

// my types;
struct Super_t {
    unsigned char bitmap[BLOCK_SIZE];
};

struct Dir_t {
    unsigned int magic;
    unsigned int num_entries;
    struct {
        char name[MAX_F_NAME_SIZE + 1];
        short block_num;
    } dir_entries[MAX_DIR_ENTRIES];
    short blocks[MAX_DATA_BLOCKS];
};
struct Inode_t {
    unsigned int magic;
    unsigned int size;
    short blocks[MAX_DATA_BLOCKS];
};

struct Data_t {
    char data[BLOCK_SIZE];
};



