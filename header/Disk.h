// Здесь реализована имитация диска, состоящего из массива блоков.

#pragma once
#include "Blocks.h"

class Disk {
    int fd;
public:
    bool mount(const char *);

    void unmount() const;

    void read_block(int, void *) const;

    void write_block(int, void *) const;

};