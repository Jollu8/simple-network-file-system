//my headers;
#include "Disk.h"
#include "Blocks.h"
#include "Basic.h"

void Basic::mount() {
    bool new_disk = disk.mount("DISK");
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
