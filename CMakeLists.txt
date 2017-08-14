cmake_minimum_required (VERSION 2.8)
project (wv_shm_test)
subdirs (common)
include_directories(common)
include_directories(./)
add_executable(wv_shm_test test_main.c wv_shm_mngt.c)

target_link_libraries(wv_shm_test wv_shm_common)

