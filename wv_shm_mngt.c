#include "wv_shm_mngt.h"


#define SHMMAX 5000
#define SHMMIN 1000

typedef struct wv_packet{
  int packet_id;
  char data[8192];
}wv_packet_t;

wv_shm_meta_t shm_meta;

int wv_shm_init ()
{
  int ret = 0;
  void* shm_addr = (void*)0;
  void* cur_addr = NULL;


  int i, j, shm_alloc_size, page_size  = 0;
  int shm_id = 0;

  struct shmid_ds shm_info;

  shm_meta.count = 0;
  shm_meta.shm_key = SHM_KEY;
  shm_meta.shm_junk_size = SHM_JUNK_SIZE;

  /* Remove previous shared memory */
  if ((shm_id = shmget(shm_meta.shm_key, 0, 0)) != -1){
    fprintf(stderr,
	    "[Success] Shared memory was existing.... (shm_id : %d, errno : %s)\n",
	    shm_id, strerror(errno));
    shmctl(shm_id, IPC_RMID, 0);
  }
  
  /* Getting shared memory */
  if ((shm_id = shmget(shm_meta.shm_key,
		       SHM_TOTAL_SIZE,
  		       IPC_CREAT|0777)) == -1)
  {
    perror( "shmget" );
    exit( errno );
    fprintf(stderr,
  	    "Getting Shared memory was failed..... (shm_key : %d, errno : %s)\n",
  	    shm_meta.shm_key, strerror(errno));
    ret = -1;
    goto shm_init_ret;
  }


  /* Attaching shared memory. */
  shm_addr = shmat(shm_id, (void*)0, 0);
  if ( shm_addr == (void*)(-1) )
  {
    fprintf(stderr,
  	    "Getting shared memory address was failed.... (shm_id : %d, errno : %s)\n",
  	    shm_id, strerror(errno));
    ret = -1;
    goto shm_init_ret;
  }

  /* Setting shared memory meta file. */
  shm_meta.shm_start_addr = shm_addr;
  page_size = getpagesize();
  shm_alloc_size = (page_size) * page_size + page_size;
  shm_meta.shm_end_addr = shm_meta.shm_start_addr + shm_alloc_size;
  shm_meta.shm_total_size = shm_alloc_size;

  printf("Shared memory start : %p\n", shm_meta.shm_start_addr);
  printf("Shared memory end : %p\n", shm_meta.shm_end_addr);
  printf("Shared memory total size : %d\n", shm_meta.shm_total_size);

  if (shmctl(shm_id, IPC_STAT, &shm_info) == -1){
    printf( "공유 메모리 정보 구하기에 실패했습니다.\n");
    return -1;
  }

  printf( "페이지 사이즈 : %d\n", getpagesize());
  printf( "공유 메모리를 사용하는 프로세스의 개수 : %ld\n", shm_info.shm_nattch);
  printf( "공유 메모리를 사이즈 : %ld\n", shm_info.shm_segsz);

  cur_addr = shm_meta.shm_start_addr;
  if(wv_shm_wr_elem(cur_addr, &shm_meta, sizeof(wv_shm_meta_t), &cur_addr)){
    printf("shm_meta start addr : %p\n", shm_meta.shm_start_addr);
    printf("cur_addr : %p\n", cur_addr);
  }

  wv_shm_meta_t* shm_meta_test = (wv_shm_meta_t*)shm_meta.shm_start_addr;
  printf("shm_meta_test->shm_start_addr : %p\n", shm_meta_test->shm_start_addr);
  printf("shm_meta_test->shm_end_addr : %p\n", shm_meta_test->shm_end_addr);
  printf("shm_meta_test->shm_total_addr : %d\n", shm_meta_test->shm_total_size);


  /* Split the section of shared memory */
  for (i=0; i<SHM_MAX_COUNT; i++){
    shm_meta.arr_shm_junk_bndry[i].start_addr = cur_addr;
    shm_meta.arr_shm_junk_bndry[i].end_addr = cur_addr + SHM_JUNK_SIZE;
    cur_addr += SHM_JUNK_SIZE;
  }

  wv_shm_junk_bndry_info_t* shm_junk_info = &(shm_meta.arr_shm_junk_bndry[0]);
  printf("shm_junk_info->start_addr : %p\n", shm_junk_info->start_addr);
  printf("shm_junk_info->end_addr : %p\n", shm_junk_info->end_addr);

  void* junk_cur_addr = shm_junk_info->start_addr;
  void* junk_next_addr = NULL;

  for(i=0; i<10; i++){
    wv_packet_t packet;
    packet.packet_id = i;
    snprintf(packet.data, 8192, "%s%d", packet.data, packet.packet_id);

    wv_shm_junk_elem_hdr_t shm_junk_elem;
    shm_junk_elem.attr_count=0;
    shm_junk_elem.attrs = NULL;

    for(j=0; j<5; j++){
      printf("into j\n");
      wv_shm_junk_elem_attr_t attr;

      attr.attr_size = sizeof(wv_packet_t);

      snprintf(attr.key, 256, "kkk%d", i);
      attr.data = (void*)malloc(sizeof(wv_packet_t));
      memcpy(attr.data, &packet, sizeof(wv_packet_t));

      if(shm_junk_elem.attrs){
	printf("we are in realloc section\n");
	shm_junk_elem.attrs = realloc(shm_junk_elem.attrs , sizeof(shm_junk_elem.attrs) + sizeof(wv_shm_junk_elem_attr_t));
      }
      else{
	printf("we are in malloc section\n");
	shm_junk_elem.attrs = (wv_shm_junk_elem_attr_t*)malloc(sizeof(wv_shm_junk_elem_attr_t));
      }

      printf("Usable size : %ld\n", malloc_usable_size(shm_junk_elem.attrs));
      /* printf("size of shm_junk_elem.attrs : %ld", strlen(shm_junk_elem.attrs)); */
    }

    /* attr.attr_size = sizeof(wv_packet_t); */

    /* shm_elem = sizeof(wv_packet_t); */
    /* shm_elem.data = (void*)malloc(sizeof(packet)); */
    /* shm_elem.data = junk_cur_addr; */
    /* shm_elem.next_addr = NULL; */

    /* wv_shm_wr_elem_limit(junk_cur_addr, shm_) */
  }

 shm_init_ret:

  return ret;
}


void* wv_shm_wr_elem(void* start_addr, void* data, size_t size, void** next_addr)
{
  void* alloc_addr = NULL;

  if (start_addr && data){
    if ((alloc_addr = memcpy(start_addr, data, size))){
      *next_addr = alloc_addr + size;
    }
    else{
      fprintf(stderr,
	      "memset() was failed.... (errono : %s)\n",
	      strerror(errno));
    }
  }
  return alloc_addr;
}


void* wv_shm_wr_elem_limit(void* start_addr, void* data, size_t size, void** next_addr, void* limit_addr){

  void* alloc_addr = NULL;
  void* addr_added_size = NULL;

  if (start_addr && data && limit_addr){
    addr_added_size = start_addr + size;
    if (alloc_addr < limit_addr){
      if ((alloc_addr = wv_shm_wr_elem(start_addr, data, size, next_addr))){ 
      }
      else{
	fprintf(stderr, "write shm was failed....(errono : %s)\n", strerror(errno));
      }
    }
    else{
      fprintf(stderr, "write shm was failed....(errono : %s)\n", strerror(errno));
    }
  }

  return alloc_addr;
}


int shm_load_config(){
  int ret = 0;
  return ret;
}


