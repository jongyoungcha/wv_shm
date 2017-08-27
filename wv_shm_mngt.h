#ifndef _WV_SHM_MNGT_H
#define _WV_SHM_MNGT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <sys/ipc.h>
#include <wv_file.h>
#include <wv_log.h>

#define SHM_KEY 4000
#define SHM_SMPR_KEY 3000
#define PROC_LIMIT 65536

#define SHM_TOTAL_SIZE (1000 * 1024 * 1024)
#define SHM_JUNK_SIZE (1 * 1024 * 1024)
#define SHM_MAX_COUNT 100

enum {
  SHM_INIT = 1,
  SEMA_INIT = 2,
};


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
  size_t startoffs;
  size_t quu_startoffs;
  size_t quu_endoffs;
  size_t writeoffs;
  size_t prev_writeoffs;  // Used to link currnet element and prev element.
  size_t readoffs;
  size_t remainsz;
  size_t count;
} wv_shm_junk_hdr_t;


typedef struct wv_shm_meta{
  int test_value;
  size_t shm_key;
  size_t shm_totalsize;
  size_t shm_junk;
  /* void* shm_startaddr[PROC_LIMIT]; */
  void* shm_endaddr;
  size_t count;
  size_t arr_junkhdr_offsets[SHM_MAX_COUNT];
} wv_shm_meta_t;


typedef struct wv_shm_junk_elem_hdr{
  size_t size;
  size_t nextoffs;
} wv_shm_junk_elem_hdr_t;


typedef union wv_shm_quusmphr{
  int val;
  struct semid_ds *buf;
  unsigned short int *array;
} wv_shm_quusmphr_t;


extern int
wv_shm_init(int flags);

extern int
wv_shm_init_shm(int flags);

extern int
wv_shm_load_meta(void* shm_startaddr);

extern int
wv_shm_init_meta(void* shm_startaddr);

extern int
wv_shm_init_smphr();

extern int
wv_shm_sync_meta();

extern int
wv_shm_init_smphr(int flags);

extern int
wv_shm_lock_quu(int index);

extern int
wv_shm_unlock_quu(int index);

extern wv_shm_junk_hdr_t*
wv_shm_assign_junk(const char* junk_name);

extern wv_shm_junk_hdr_t*
wv_shm_unassign_junk(const char* junkname);

extern int
wv_shm_get_junk_index(wv_shm_junk_hdr_t* junkhdr);

extern int
wv_shm_get_junk_count();

extern void*
wv_shm_wr(size_t start_offs, void* data, size_t size, size_t *next_offs);

extern int
wv_shm_junk_init(char* shm_junk_name, size_t start_offs, size_t end_offs);

extern void*
wv_shm_rd(void* start_addr, size_t size, void** next_addr);

extern int
wv_shm_clear_junk(wv_shm_junk_hdr_t* shm_junk_hdr);

extern wv_shm_junk_hdr_t**
wv_shm_get_junk_list();

extern void
wv_shm_free_junk_list(wv_shm_junk_hdr_t** junk_hdr_list);

extern wv_shm_junk_elem_hdr_t*
wv_shm_peek_elem_hdr(wv_shm_junk_hdr_t* shm_junk_hdr);

extern void*
wv_shm_peek_elem_data(wv_shm_junk_hdr_t* shm_junk_hdr);

extern void*
wv_shm_pop_elem_data(wv_shm_junk_hdr_t* shm_junk_hdr, wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr);

extern int
wv_shm_push_elem(wv_shm_junk_hdr_t* shm_junk_hdr, void* data, size_t size);

extern wv_shm_junk_hdr_t*
wv_shm_find_junk(const char* junk_name);

extern int
wv_shm_dump_junk(const char* junk_name, const char* dir_name);

extern wv_shm_junk_hdr_t*
wv_shm_load_junk(const char* dir_name, const char* file_name);

int
wv_shm_link_elems(wv_shm_junk_hdr_t* junk_hdr, size_t prev_offset, size_t cur_offset);

extern void
wv_shm_show_junk(wv_shm_junk_hdr_t* shm_junk_hdr);

int wv_shm_check_init();

#endif
