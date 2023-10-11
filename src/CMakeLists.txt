cmake_minimum_required(VERSION 3.10)
project(NFS) # Замените на имя вашего проекта

set(CMAKE_CXX_STANDARD 17)

# Указываем CMake где искать заголовочные файлы
include_directories(${PROJECT_SOURCE_DIR}/../header)

# Указываем CMake где искать исходные файлы
file(GLOB SOURCES Wrapped.cpp "*.cpp")

add_library(NFS ${SOURCES})