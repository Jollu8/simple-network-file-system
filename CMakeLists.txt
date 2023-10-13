cmake_minimum_required(VERSION 3.23)
project(NFS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

add_subdirectory(src)

add_executable(nfsserver Server.cpp)
target_link_libraries(nfsserver NFS)

add_executable(nfsclient  Client.cpp)
target_link_libraries(nfsclient NFS)

add_custom_command(TARGET nfsserver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove -f DISK)

set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/nfsserver"
        "${CMAKE_CURRENT_BINARY_DIR}/nfsclient"
        "${CMAKE_CURRENT_BINARY_DIR}/*.o"
        "${CMAKE_CURRENT_BINARY_DIR}/DISK"
        )

# Настройка CPack
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
install(TARGETS nfsserver nfsclient
        RUNTIME DESTINATION bin)

include(CPack)
