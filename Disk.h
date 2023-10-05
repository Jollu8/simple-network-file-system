// Здесь реализована имитация диска, состоящего из массива блоков.

#pragma once

class Disk {
    int fd;
public:
    bool mount(const char *);

    void unmount();

    void read_block(int, void *);

    void write_block(int, void *);

};