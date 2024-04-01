cmake_minimum_required(VERSION 3.10)
project(co VERSION 1.0)

enable_language(C ASM)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_FILES
        main.cpp
        coroutine.cpp
        co_swap.S)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(co ${SOURCE_FILES})