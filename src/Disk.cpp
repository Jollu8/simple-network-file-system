#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>


// my header
#include "Disk.h"


// Открывает файл "имя_файла", представляющий собой диск.  Если файл
// не существует, файл создается. Возвращает true, если файл создан, и false, если
// параметр файла fd существует. Любая другая ошибка прерывает работу программы.
bool Disk::mount(const char *fileName) {
    fd = open(fileName, O_RDWR);
    if (fd != -1) return false;
    fd = open(fileName, O_RDWR | O_CREAT | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        std::cerr << "Could not creat disk" << std::endl;
        exit(-1);
    }
    return true;
}

void Disk::unmount() const {
    close(fd);
}

void Disk::read_block(int blockNum, void *block) const {
    off_t offset;
    off_t new_offset;
    ssize_t size_;
    if (blockNum < 0 || blockNum >= NUM_BLOCKS) {
        std::cerr << "Invalid block size" << std::endl;
        exit(-1);
    }

    offset = blockNum * BLOCK_SIZE;
    new_offset = lseek(fd, offset, SEEK_SET);
    if (offset != new_offset) {
        std::cerr << "Seek failure" << std::endl;
        exit(-1);
    }
    size_ = read(fd, block, BLOCK_SIZE);
    if (size_ != BLOCK_SIZE) {
        std::cerr << "Failed to read entire block" << std::endl;
        exit(-1);
    }
}

void Disk::write_block(int blockNum, void *block) const {
    off_t offset;
    off_t new_offset;
    ssize_t size_;

    if (blockNum < 0 || blockNum >= NUM_BLOCKS) {
        std::cerr << "Invalid block size" << std::endl;
        exit(-1);
    }
    offset = blockNum * BLOCK_SIZE;
    new_offset = lseek(fd, offset, SEEK_SET);
    if (offset != new_offset) {
        std::cerr << "Seek failure" << std::endl;
        exit(-1);

    }

    size_ = write(fd, block, BLOCK_SIZE);
    if (size_ != BLOCK_SIZE) {
        std::cerr << "Failed to write entire block" << std::endl;
        exit(-1);

    }
}

