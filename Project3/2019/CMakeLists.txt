cmake_minimum_required(VERSION 3.15)
project(io_multithreading C)

find_package (Threads)

set(CMAKE_C_STANDARD 99)

add_executable(io_multithreading main.c lib/common_libs.h lib/utils.h lib/thread_factory.h)

target_link_libraries (io_multithreading ${CMAKE_THREAD_LIBS_INIT})