cmake_minimum_required(VERSION 3.28)
project(bsServer C)

set(CMAKE_C_STANDARD 23)

add_executable(bsServer main.c
        keyValStore.c
        keyValStore.h
        sub.h
        sub.c
        helper.h
        helper.c)
