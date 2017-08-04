#include "wv_shm_mngt.h"


int shm_init (wv_file_t* meta_file, const char* meta_dir_path)
{
  int ret = 0;

  return ret;
}


int shm_add_mem (int shm_id, char* shm_name, size_t data_size, size_t max_count)
{
  int ret = 0;
  void* shm_addr = (void*)0;

  shm_elem_t* elem = (shm_elem_t*)malloc(sizeof(shm_elem_t));

  printf("shm_elem_size : %ld\n", sizeof(shm_elem_t));
  printf("sizeof int : %ld\n", sizeof(int));
  printf("sizeof void* : %ld\n", sizeof(void*));
  printf("shm_elem_size : %ld\n", sizeof(shm_elem_t));

 
  shm_hdr_t shm_hdr; 
  shm_hdr.count = 0;
  shm_hdr.data_size = data_size;
  shm_hdr.maximum = max_count;
  shm_hdr.read_pos = 0;
  shm_hdr.write_pos = 0;

  if (shmget(shm_id, 0, 0) != -1){
    fprintf(stderr,
	    "shared memory was existing.... (shm_id : %d, errno : %s)\n",
	    shm_id, strerror(errno));
  }

  if (shmget(shm_id,
	     (sizeof(shm_hdr) + (sizeof(elem) + sizeof(data_size)) * max_count),
	     IPC_EXCL|IPC_CREAT) == -1){
    fprintf(stderr,
	    "making shared memory was failed.... (shm_id : %d, errno : %s)\n",
	    shm_id, strerror(errno));
    ret = -1;
    goto shm_add_mem_ret;
  }

  shm_addr = shmat(shm_id, (void*)0, SHM_RND);
  if (shm_addr == (void*)(-1)){

    fprintf(stderr,
	    "getting shared memory address was failed.... (shm_id : %d, errno : %s)\n",
	    shm_id, strerror(errno));
    ret = -1;
    goto shm_add_mem_ret;
  }

  printf("getting shared memory addr : %p", shm_addr);

 shm_add_mem_ret:
  return ret;
}


int shm_wr_elem(int shm, int pos, void* data, size_t size)
{
  int ret = 0;

  shm_elem_t* elem = (shm_elem_t*)malloc(sizeof(shm_elem_t));
  elem -> is_empty = 1;
  elem -> elem_data = data;
  elem -> padding = 0x00000000;
  printf("size of elem : %ld\n", sizeof(elem));



 shm_wr_elem_ret:
  return ret;
}


