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
  int shm_alloc_size = 0; 
  int new_shm_id = 0;
  struct shmid_ds shm_info;

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
	    "[Success] shared memory was existing.... (shm_id : %d, errno : %s)\n",
	    shm_id, strerror(errno));
  }

  shm_alloc_size =  sizeof(shm_hdr) + (sizeof(elem) + sizeof(data_size)) * max_count;
  printf("shm_alloc_size : %d", shm_alloc_size);
      
  if ((new_shm_id = shmget(shm_id,
			(sizeof(shm_hdr) + (sizeof(elem) + sizeof(data_size)) * max_count),
			IPC_CREAT|0777)) == -1){
    fprintf(stderr,
	    "making shared memory was failed.... (shm_id : %d, errno : %s)\n",
	    shm_id, strerror(errno));
    ret = -1;
    goto shm_add_mem_ret;
  }
  printf("new shm_id %d\n", new_shm_id);

  shm_addr = shmat(new_shm_id, (void*)0, 0);
  printf("shm_addr : %p\n", shm_addr);
  if (shm_addr == (void*)(-1)){

    fprintf(stderr,
	    "getting shared memory address was failed.... (shm_id : %d, errno : %s)\n",
	    new_shm_id, strerror(errno));
    ret = -1;
    goto shm_add_mem_ret;
  }

  printf("getting shared memory addr : %p\n", shm_addr);

  if ( -1 == shmctl( new_shm_id, IPC_STAT, &shm_info))
   {
      printf( "공유 메모리 정보 구하기에 실패했습니다.\n");
      return -1;
   }

   printf( "페이지 사이즈 : %d\n", getpagesize());
   printf( "공유 메모리를 사용하는 프로세스의 개수 : %d\n", shm_info.shm_nattch);
   printf( "공유 메모리를 사이즈 : %ld\n", shm_info.shm_segsz);

   if ( -1 == shmdt( shm_addr))
   {
      printf( "공유 메모리 분리 실패\n");
      return -1;
   }
   else
   {
      printf( "공유 메모리 분리 성공\n");
   }

 shm_add_mem_ret:
  return ret;
}



int shm_wr_elem(int shm_id, int pos, void* data, size_t size)
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


