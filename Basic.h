////  Implements low-level file system functionality that interfaces with the disk.

#pragma once

// my headers;
#include "Disk.h"

class Basic {
public:
    void mount();

    void unmount();

    short get_free_block();

    void reclaim_block(short);

    void read_block(short, void *);

    void write_block(short, void *);

private:
    Disk disk;
};

