cmake_minimum_required(VERSION 3.10)
project(NFS) # Замените на имя вашего проекта

set(CMAKE_CXX_STANDARD 20)

# Указываем CMake где искать заголовочные файлы
include_directories(${PROJECT_SOURCE_DIR}/../header)

# Указываем CMake где искать исходные файлы
file(GLOB SOURCES "*.cpp" Shell.cpp)

add_library(NFS ${SOURCES}) # Замените на имя вашего исполняемого файла
