cmake_minimum_required(VERSION 3.10)
project(utils C)

set(CMAKE_C_STANDARD 99)

add_executable(utils main.c ../convert_utils.c ../convert_utils.h)