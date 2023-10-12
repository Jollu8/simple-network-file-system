//my headers;

#include "Basic.h"

void Basic::mount() {
    bool new_disk = disk.mount("jollu");
    if (!new_disk) return;
    Super_t super_block;
    super_block.bitmap[0] = 0x3;
    for (auto i = 0; i < BLOCK_SIZE; ++i) super_block.bitmap[i] = 0;

    disk.write_block(0, (void *) &super_block);
    Dir_t dir_block{};
    dir_block.magic = DIR_MAGIC_NUM;
    dir_block.num_entries = 0;
    for (auto i = 0; i < MAX_DIR_ENTRIES; ++i) dir_block.dir_entries[i].block_num = 0;

    disk.write_block(1, (void *) &dir_block);

    Data_t data_block;
    for (auto i = 0; i < BLOCK_SIZE; ++i) data_block.data[i] = 0;
    for (auto i = 2; i < NUM_BLOCKS; ++i) disk.write_block(i, (void *) &data_block);


}

void Basic::unmount() {
    disk.unmount();

}

short Basic::get_free_block() {
    Super_t super_block_t;
    disk.read_block(0, (void *) &super_block_t);

    for (auto byte = 0; byte < BLOCK_SIZE; ++byte) {
        if (super_block_t.bitmap[byte] != 0xFF) {
            for (int bit = 0; bit < 8; ++bit) {
                int mask = 1 << bit;
                if (mask & ~super_block_t.bitmap[byte]) {
                    super_block_t.bitmap[byte] |= mask;
                    disk.write_block(0, (void *) &super_block_t);
                    return (byte * 8) + bit;
                }
            }
        }
    }
    return 0;
}

void Basic::reclaim_block(short block_num) {
    Super_t super_block;
    disk.read_block(0, (void *) &super_block);
    int byte = block_num / 8;
    int bit = block_num % 8;
    unsigned char mask = ~(1 << bit);
    super_block.bitmap[byte] &= mask;
    disk.write_block(0, (void *) &super_block);

}

void Basic::read_block(short block_num, void *block) {
    disk.read_block(block_num, block);

}

void Basic::write_block(short block_num, void *block) {
    disk.write_block(block_num, block);

}