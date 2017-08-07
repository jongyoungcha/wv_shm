#include "wv_shm_mngt.h"


#define SHMMAX 5000
#define SHMMIN 1000

wv_shm_meta_t shm_meta;

int wv_shm_init ()
{
  int ret = 0;
  void* shm_addr = (void*)0;

  int i, shm_alloc_size, page_size  = 0;
  int shm_id = 0;

  struct shmid_ds shm_info;

  shm_meta.count = 0;
  shm_meta.shm_key = SHM_KEY;
  shm_meta.shm_total_size = SHM_TOTAL_SIZE;
  shm_meta.shm_junk_size = SHM_JUNK_SIZE;

  for(i=0; i<SHM_MAX_COUNT; i++){
  }

  /* Remove previous shared memory */
  if ((shm_id = shmget(shm_meta.shm_key, 0, 0)) != -1){
    fprintf(stderr,
	    "[Success] Shared memory was existing.... (shm_id : %d, errno : %s)\n",
	    shm_id, strerror(errno));
    shmctl(shm_id, IPC_RMID, 0);
  }
  
  /* Getting shared memory */
  if ((shm_id = shmget(shm_meta.shm_key,
		       1024*1024*1024,
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
  printf("shm_addr : %p\n", shm_addr);

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
  shm_alloc_size = (shm_meta.shm_total_size * 1024 * 1024 / page_size) * page_size + page_size;
  shm_meta.shm_end_addr = shm_meta.shm_start_addr + shm_alloc_size;

  printf("Shared memory alloc size : %d\n", shm_alloc_size);
  printf("Shared memory start : %p\n", shm_meta.shm_start_addr);
  printf("Shared memory end : %p\n", shm_meta.shm_end_addr);

  if (shmctl(shm_id, IPC_STAT, &shm_info) == -1){
    printf( "공유 메모리 정보 구하기에 실패했습니다.\n");
    return -1;
  }

   printf( "페이지 사이즈 : %d\n", getpagesize());
   printf( "공유 메모리를 사용하는 프로세스의 개수 : %ld\n", shm_info.shm_nattch);
   printf( "공유 메모리를 사이즈 : %ld\n", shm_info.shm_segsz);

   /* Split the section of shared memory */
   


 shm_init_ret:
   
  return ret;
}

int shm_load_config(){
  int ret = 0;
  return ret;
}


/* int shm_add_mem (int shm_id, char* shm_name, size_t data_size, size_t max_count) */
/* { */
/*   int ret = 0; */
/*   void* shm_addr = (void*)0; */
/*   int shm_alloc_size = 0;  */
/*   int new_shm_id = 0; */
/*   struct shmid_ds shm_info; */

/*   printf("shm_elem_size : %ld\n", sizeof(wv_shm_elem_t)); */
/*   printf("sizeof int : %ld\n", sizeof(int)); */
/*   printf("sizeof void* : %ld\n", sizeof(void*)); */
/*   printf("shm_elem_size : %ld\n", sizeof(wv_shm_elem_t)); */
 

/*   if (shmget(shm_id, 0, 0) != -1){ */
/*     fprintf(stderr, */
/* 	    "[Success] Shared memory was existing.... (shm_id : %d, errno : %s)\n", */
/* 	    shm_id, strerror(errno)); */
/*   } */

/*   shm_alloc_size =  sizeof(shm_hdr) + (sizeof(elem) + sizeof(data_size)) * max_count; */
/*   printf("shm_alloc_size : %d", shm_alloc_size); */
      
/*   if ((new_shm_id = shmget(shm_id, */
/* 			(sizeof(shm_hdr) + (sizeof(elem) + sizeof(data_size)) * max_count), */
/* 			IPC_CREAT|0777)) == -1){ */
/*     fprintf(stderr, */
/* 	    "Making shared memory was failed.... (shm_id : %d, errno : %s)\n", */
/* 	    shm_id, strerror(errno)); */
/*     ret = -1; */
/*     goto shm_add_mem_ret; */
/*   } */
/*   printf("new shm_id %d\n", new_shm_id); */

/*   shm_addr = shmat(new_shm_id, (void*)0, 0); */
/*   printf("shm_addr : %p\n", shm_addr); */

/*   if (shm_addr == (void*)(-1)){ */
/*     fprintf(stderr, */
/* 	    "Getting shared memory address was failed.... (shm_id : %d, errno : %s)\n", */
/* 	    new_shm_id, strerror(errno)); */
/*     ret = -1; */
/*     goto shm_add_mem_ret; */
/*   } */

/*   printf("Getting shared memory addr : %p\n", shm_addr); */

/*   if ( -1 == shmctl( new_shm_id, IPC_STAT, &shm_info)) */
/*    { */
/*       printf( "공유 메모리 정보 구하기에 실패했습니다.\n"); */
/*       return -1; */
/*    } */

/*    printf( "페이지 사이즈 : %d\n", getpagesize()); */
/*    printf( "공유 메모리를 사용하는 프로세스의 개수 : %ld\n", shm_info.shm_nattch); */
/*    printf( "공유 메모리를 사이즈 : %ld\n", shm_info.shm_segsz); */

/*    /\* if ( -1 == shmdt( shm_addr)) *\/ */
/*    /\* { *\/ */
/*    /\*    printf( "공유 메모리 분리 실패\n"); *\/ */
/*    /\*    return -1; *\/ */
/*    /\* } *\/ */
/*    /\* else *\/ */
/*    /\* { *\/ */
/*    /\*    printf( "공유 메모리 분리 성공\n"); *\/ */
/*    /\* } *\/ */

/*  shm_add_mem_ret: */
/*   return ret; */
/* } */



/* int wv_shm_wr_elem(int shm_id, wv_shm_meta_node_t** node, int pos, void* data, size_t size) */
/* { */
/*   int ret = 0; */

/*   wv_shm_elem_t* elem = (wv_shm_elem_t*)malloc(sizeof(wv_shm_elem_t)); */
/*   elem -> is_empty = 1; */
/*   elem -> elem_data = data; */
/*   elem -> padding = 0x00000000; */
/*   printf("size of elem : %ld\n", sizeof(elem)); */

/*   if(node == NULL){ */
/*   } */

/*  shm_wr_elem_ret: */
/*   return ret; */
/* } */



/* int wv_shm_init_meta_tree(wv_shm_meta_node_t** header, int key, wv_shm_meta_node_t* node, size_t data_size){ */

/*   int ret = 0; */

/*   if((*header) == NULL){ */
/*     ret = -1; */
/*     fprintf(stderr, */
/* 	    "wv_shm_init_meta_tree() failed... (errno : %s)\n", strerror(errno)); */
/*     goto  wv_shm_init_meta_tree_ret: */
/*   } */

/*  wv_shm_init_meta_tree_ret: */

/*   return ret; */
/* } */




/* int wv_shm_init_meta_node(wv_shm_meta_node_t** node, int shm_id, char* shm_name, void* shm_addr, int shm_length){ */
/*   int ret = 0; */

/*   if(node == NULL){ */
/*     *node = (wv_shm_meta_node_t*)malloc(sizeof(wv_shm_meta_node_t)); */
/*   } */

/*   (*node) -> shm_id = shm_id; */
/*   snprintf((*node)->shm_name, sizeof((*node)->shm_name), "%s", shm_name); */

/*   (*node)->shm_length = shm_length; */
/*   (*node)->shm_stat_addr = shm_addr; */
/*   (*node)->left = NULL; */
/*   (*node)->right = NULL; */

/*   return ret; */
/* } */



/* int wv_shm_meta_tree_find_loc(wv_shm_meta_node_t* start_node, */
/* 			 int shm_id_key, */
/* 			 wv_shm_meta_node_t** loc_to_write) */
/* { */
/*   wv_shm_meta_node_t* cur_node = NULL; */
/*   int ret = 0; */

/*   cur_node = start_node; */

/*   if (cur_node && cur_node-> shm_id < shm_id_key){ */
/*     cur_node = cur_node->right ? start_node ->right : NULL; */
/*     if (cur_node == NULL) { */
/*       ret = 1; */
/*       goto wv_shm_find_loc_meta_ret; */
/*     }else{ */
/*       if (( ret = wv_shm_meta_tree_find_loc(cur_node, shm_id_key, loc_to_write) ) == 1){ */
/* 	goto wv_shm_find_loc_meta_ret; */
/*       } */
/*     } */
/*   } */
/*   else if (start_node->shm_id > shm_id_key){ */
/*     cur_node = cur_node->left ? start_node ->left : NULL; */
/*     if (cur_node == NULL) { */
/*       ret = 1; */
/*       goto wv_shm_find_loc_meta_ret; */
/*     } */
/*     else{ */
/*       if (( ret = wv_shm_meta_tree_find_loc(cur_node, shm_id_key, loc_to_write) ) == 1){ */
/* 	goto wv_shm_find_loc_meta_ret; */
/*       } */
/*     } */
/*   } */
/*   else { */
/*     *loc_to_write = NULL; */
/*     ret = 0; */
/*     goto wv_shm_find_loc_meta_ret; */
/*   } */

/*  wv_shm_find_loc_meta_ret: */

/*   return ret; */

/* } */




