cmake_minimum_required(VERSION 3.7.2)
project(UDSP_semestralka_DiskusnePrispevky C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)

add_executable(UDSP_semestralka_DiskusnePrispevky-server server.c helpers.c helpers.h server.h)
add_executable(UDSP_semestralka_DiskusnePrispevky-klient klient.c helpers.c helpers.h klient.h)