cmake_minimum_required(VERSION 3.10)

project(TCC
    VERSION 0.1
    DESCRIPTION "Tiny C Compiler"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(libvarvm)
add_subdirectory(libvmasm)
add_subdirectory(libvmcore)
add_subdirectory(tcc)
add_subdirectory(vasm)
add_subdirectory(vexe)
add_subdirectory(vm)
