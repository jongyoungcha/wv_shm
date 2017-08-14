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


/* +-----------------------------+ */
/* | A wv_shm structure.         | */
/* +-----------------------------+ */
/* |                 shm_mem     | */
/* |                /  |   \     | */
/* |        shm_junk  ..  ..     | */
/* |        /  |  \              | */
/* |  shm_elem ..  ..            | */
/* +-----------------------------+ */

/* +--------------------------------------------------------------+ */
/* | A junk memory structure                                      | */
/* +--------------------------------------------------------------+ */
/* | junk_hdr | junk_elem_hdr | data | junk_elem_hdr | data | ... | */
/* +--------------------------------------------------------------+ */

typedef struct wv_shm_junk_hdr{
  char shm_name[256];
  int is_assigned;
  size_t start_offset;
  size_t quu_start_offset;
  size_t quu_end_offset;
  size_t write_offset;
  size_t prev_write_offset;  // Used to link currnet element and prev element.
  size_t read_offset;
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


typedef struct wv_shm_junk_elem_hdr{
  size_t size;
  size_t next_offset;
} wv_shm_junk_elem_hdr_t;


/* typedef struct wv_shm_junk_elem_attr{ */
/*   size_t attr_size; */
/*   size_t key_size; */
/*   void* data; */
/*   void* key; */
/* } wv_shm_junk_elem_attr_t; */




int
wv_shm_test ();

extern int
wv_shm_init();

extern int
wv_shm_load_meta(void* shm_start_addr);

extern int
wv_shm_init_meta(void* shm_start_addr);

extern int
wv_shm_sync_meta();

extern wv_shm_junk_hdr_t*
wv_shm_assign_junk(const char* junk_name);

extern wv_shm_junk_hdr_t*
wv_shm_unassign_junk(wv_shm_meta_t* shm_meta, const char* junk_name);

int
wv_shm_chk_offs(size_t offset);

extern void*
wv_shm_wr(size_t start_offs, void* data, size_t size, size_t *next_offs);

extern int
wv_shm_junk_init(wv_shm_junk_hdr_t* shm_junk_hdr, char* shm_junk_name, size_t start_offs, size_t end_offs);

void*
wv_shm_rd(void* start_addr, size_t size, void** next_addr);

extern int
wv_shm_clear_junk(wv_shm_junk_hdr_t* shm_junk_hdr);

extern wv_shm_junk_hdr_t**
wv_shm_get_junk_list();

extern void
wv_shm_free_junk_list(wv_shm_junk_hdr_t** junk_hdr_list);

/* extern wv_shm_junk_hdr_t* */
/* wv_shm_find_junk(wv_shm_meta_t* shm_meta, const char* junk_name); */

/* extern int */
/* wv_shm_dump_junk(const char* junk_name, const char* dir_name, const char* file_name); */

/* extern int */
/* wv_shm_load_junk(wv_shm_meta_t* shm_meta, const char* dir_name, const char* file_name); */

extern wv_shm_junk_elem_hdr_t*
wv_shm_peek_elem_hdr(wv_shm_junk_hdr_t* shm_junk_hdr);

/* extern void* */
/* wv_shm_peek_elem_data(wv_shm_junk_hdr_t* shm_junk_hdr); */

extern void*
wv_shm_pop_elem_data(wv_shm_junk_hdr_t* shm_junk_hdr, wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr);

/* extern int */
/* wv_shm_del_last_elem(wv_shm_junk_hdr_t* shm_junk_hdr); */


extern void*
wv_shm_push_elem(wv_shm_junk_hdr_t* shm_junk_hdr, void* data, size_t size);



int
wv_shm_link_elems(wv_shm_junk_hdr_t* junk_hdr, size_t prev_offset, size_t cur_offset);


extern void
wv_shm_junk_show(wv_shm_junk_hdr_t* shm_junk_hdr);



#endif
