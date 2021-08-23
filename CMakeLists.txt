cmake_minimum_required(VERSION 3.17)
project(SERVER C)

set(CMAKE_C_STANDARD 99)

add_executable(SERVER main.c cJSON.c)

target_link_libraries(SERVER ws2_32)