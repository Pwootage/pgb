cmake_minimum_required(VERSION 3.15)
project(pgb)

set(CMAKE_CXX_STANDARD 14)

add_executable(pgb src/main.cpp src/mmu/MMU.cpp src/mmu/MMU.hpp src/rom/ROM.cpp src/rom/ROM.hpp)