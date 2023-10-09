cmake_minimum_required(VERSION 3.23)
project(simple_network_file_system)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(simple_network_file_system
        Basic.cpp
        Basic.h
        Disk.cpp
        Disk.h
        FileSys.h
        FileSys.cpp
        )
