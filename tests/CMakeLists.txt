
create_test_sourcelist(TESTS
	tests.c
	shm_init.c
	overflow.c
	data_integrity.c
	circular_integrity.c
	dump_file.c
	load_file.c)


include_directories(../)
include_directories(../common)


add_executable( tests ${TESTS} )

add_executable(init_sema init_sema.c)
add_executable(producer producer.c)
add_executable(consumer consumer.c)

target_link_libraries( tests wv_shm wv_shm_common )
target_link_libraries( init_sema wv_shm wv_shm_common )
target_link_libraries( producer wv_shm wv_shm_common )
target_link_libraries( consumer wv_shm wv_shm_common )

remove( TESTS tests.c )

foreach( test ${TESTS} )
	 get_filename_component( TName ${test} NAME_WE )
	 add_test( ${TName} tests ${TName} )

endforeach( test )
