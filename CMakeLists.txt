cmake_minimum_required (VERSION 2.8)

project (wv_shm)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

subdirs (common)

include_directories(common)
include_directories(./)

add_library(wv_shm SHARED wv_shm_mngt.c)
add_subdirectory( tests )



target_link_libraries(wv_shm wv_shm_common)

