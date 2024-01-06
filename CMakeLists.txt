cmake_minimum_required(VERSION 3.26)
project(numath C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra)

add_executable(numath
        numath.c
        numath.h
)

