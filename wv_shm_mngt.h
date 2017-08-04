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
#include <wv_file.h>


typedef struct shm_meta_node{
  int shm_id;
  char* shm_name;
  void* shm_stat_addr;
  int shm_length;
  struct shm_meta_node *left;
  struct shm_meta_node *right;
} shm_meta_node_t;


typedef struct shm_hdr{
  void* write_pos;
  void* read_pos;
  int data_size;
  int count;
  int maximum;
} shm_hdr_t;


typedef struct shm_elem{
  int is_empty;
  void* elem_data;
  void* next_node_addr;
  void* padding;
} shm_elem_t;


extern int
shm_init (wv_file_t* meta_file, const char* meta_dir_path);


extern int
shm_get_meta_nodes (struct shm_meta_node** nodes);


extern int
shm_add_mem (int shm_id, char* shm_name, size_t data_size, size_t max_count);

extern int
shm_wr_elem (int shm_id, int pos, void* data, size_t data_size);


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


#endif
