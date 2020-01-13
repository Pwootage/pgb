cmake_minimum_required(VERSION 3.15)
project(pgb)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

#message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
#message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
#message("CMAKE_CXX_FLAGS_RELWITHDEBINFO is ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
#message("CMAKE_CXX_FLAGS_MINSIZEREL is ${CMAKE_CXX_FLAGS_MINSIZEREL}")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(LIBPGB_SOURCES
        src/mmu/MMU.cpp
        src/rom/ROM.cpp
        src/cpu/CPU.cpp
        src/cpu/interpreter/interpreter.cpp
        src/cpu/interpreter/interpreter_cb.cpp
        src/gpu/GPU.cpp
        )

set(LIBPGB_HEADERS
        src/cpu/interpreter/interpreter.hpp
        src/cpu/interpreter/interpreter_cb.hpp
        )

include_directories(include)

add_executable(pgp-sdl
        main.cpp
        ${LIBPGB_SOURCES}
        ${LIBPGB_HEADERS}
        )

target_link_libraries(pgp-sdl ${SDL2_LIBRARIES})

add_library(pgb STATIC
        ${LIBPGB_SOURCES}
        ${LIBPGB_HEAERS}
        )

target_include_directories(pgb PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )
