cmake_minimum_required(VERSION 3.17)
project(soww C)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

set(CMAKE_C_STANDARD 99)

add_executable(soww lab1/pi.c lab1/integral.c lab2/lab2.c lab5/lab5.c lab5/lab5.h)
