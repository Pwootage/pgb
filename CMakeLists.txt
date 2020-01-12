cmake_minimum_required(VERSION 3.15)
project(pgb)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

#message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
#message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
#message("CMAKE_CXX_FLAGS_RELWITHDEBINFO is ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
#message("CMAKE_CXX_FLAGS_MINSIZEREL is ${CMAKE_CXX_FLAGS_MINSIZEREL}")

add_executable(pgb
        src/main.cpp
        src/mmu/MMU.cpp
        src/mmu/MMU.hpp
        src/rom/ROM.cpp
        src/rom/ROM.hpp
        src/gb_mode.hpp
        src/cpu/CPU.cpp
        src/cpu/CPU.hpp
        src/cpu/interpreter/interpreter.cpp
        src/cpu/interpreter/interpreter.hpp
        src/cpu/interpreter/interpreter_cb.cpp
        src/cpu/interpreter/interpreter_cb.hpp)