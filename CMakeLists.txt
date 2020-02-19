cmake_minimum_required(VERSION 3.10)
project(VDIFuse C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -D_FILE_OFFSET_BITS=64")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)


find_package(FUSE REQUIRED)
include_directories(${FUSE_INCLUDE_DIR})

add_executable(VDIFuse main.c include/vdi_structs.h include/vdi.h include/utils.h src/utils.c src/vdi.c include/vdif_structs.h include/ext2.h src/ext2.c include/ext2_structs.h include/DataStructures/linked_list.h src/DataStructures/linked_list.c include/DataStructures/dictionary.h src/DataStructures/dictionary.c)
target_link_libraries(VDIFuse ${FUSE_LIBRARIES})