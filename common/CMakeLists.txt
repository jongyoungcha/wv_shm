cmake_minimum_required (VERSION 2.8)
file(GLOB SOURCES
    ./*.h
    ./*.c
)
add_library(wv_shm_common SHARED ${SOURCES})
