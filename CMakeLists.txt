cmake_minimum_required(VERSION 3.16)
project(UDSP_semestralka_DiskusnePrispevky C)

set(CMAKE_C_STANDARD 99)

add_executable(UDSP_semestralka_DiskusnePrispevky-server server.c)
add_executable(UDSP_semestralka_DiskusnePrispevky-klient klient.c)