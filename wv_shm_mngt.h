#ifndef _WV_SHM_MNGT_H
#define _WV_SHM_MNGT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <sys/ipc.h>
#include <wv_file.h>
#include <wv_log.h>


#define SHM_KEY 4000
#define SHM_TOTAL_SIZE (1024 * 1024 * 1024)
#define SHM_JUNK_SIZE (100 * 1024 * 1024)
#define SHM_MAX_COUNT 100


typedef struct wv_shm_junk_hdr{
  char shm_name[256];
  void* start_addr;
  void* end_addr;
  void* write_addr;
  void* read_addr;
  size_t remain_size;
  size_t count;
} wv_shm_junk_hdr_t;


typedef struct wv_shm_meta{
  size_t shm_key;
  size_t shm_total_size;
  size_t shm_junk_size;
  void* shm_start_addr;
  void* shm_end_addr;
  size_t count;
  wv_shm_junk_hdr_t arr_shm_junk_hdr[SHM_MAX_COUNT];
} wv_shm_meta_t;


typedef struct wv_shm_junk_elem_attr{
  size_t attr_size;
  size_t key_size;
  void* data;
  void* key;
} wv_shm_junk_elem_attr_t;


typedef struct wv_shm_junk_elem_hdr{
  size_t total_size;
  size_t attr_count;
  wv_shm_junk_elem_attr_t attrs[256];
} wv_shm_junk_elem_hdr_t;


extern int
wv_shm_init();

extern int
wv_shm_junk_init(wv_shm_junk_hdr_t* shm_junk_hdr, char* shm_junk_name, void* start_addr, void* end_addr);

extern void*
wv_shm_rd_elem(wv_shm_junk_hdr_t* shm_hdr, size_t size);

extern void*
wv_shm_wr(void* start_addr, void* data, size_t size, void** next_addr, void* limit_addr);

extern void* 
wv_shm_wr_elem(wv_shm_junk_hdr_t* shm_junk_hdr, void* data, size_t size);









extern int
wv_shm_add_mem (int shm_id, char* shm_name, size_t data_size, size_t max_count);


extern int
shm_chk_id_exist (int shm_id);

extern int
shm_rm_mem (int shm_id);

extern int
shm_wr_dump_file (int shm_id, const char* shm_dump_path);

extern int
shm_ld_dump_file (int shm_id, const char* shm_dump_path);

extern int
shm_wr_dump_all();



// Tree data structure for handling the shared memory haeder and the shm_meta_data structure.
/* int */
/* wv_shm_meta_tree_init(wv_shm_meta_node_t** node, */
/* 		      int shm_id, */
/* 		      char* shm_name, */
/* 		      void* shm_addr, */
/* 		      int shm_length); */


/* int */
/* wv_shm_meta_tree_find_loc(wv_shm_meta_node_t* start_node, */
/* 			  int shm_id_key, */
/* 			  wv_shm_meta_node_t** loc_to_write); */

/* int */
/* wv_shm_make_tn(wv_shm_meta_node_t** node, char* key, void* data, size_t data_size); */

/* int */
/* wv_shm_push_tn(wv_shm_meta_node_t** header, wv_shm_meta_node_t* data); */

/* int */
/* wv_shm_srch_tn(wv_shm_meta_node_t* header, char* node_key); */

/* int */
/* wv_shm_del_tn(wv_shm_meta_node_t** header, char* node_key); */


#endif
