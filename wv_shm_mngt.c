#include "wv_shm_mngt.h"
#include <wv_mem.h>


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

  int shm_id = 0;
  int shm_exists = 0;
  int i = 0;

  struct shmid_ds shm_ds;

  memset(&shm_meta, 0x00, sizeof(shm_meta));
  shm_meta.count = 0;
  shm_meta.shm_key = SHM_KEY;
  shm_meta.shm_junk_size = SHM_JUNK_SIZE;

  /* Getting shared memory */
  if ( (shm_id = shmget(shm_meta.shm_key, 0, 0)) != -1 ){
    shm_exists = 1;
    fprintf(stderr,
	    "[Success] Shared memory was existing.... (shm_id : %d, errno : %s)\n",
	    shm_id, strerror(errno));
    /* shmctl(shm_id, IPC_RMID, 0); */
  }
  else{

    if ( (shm_id = shmget(shm_meta.shm_key,
			  SHM_TOTAL_SIZE,
			  IPC_CREAT|0777)) == -1 ){

      perror( "shmget" );
      exit( errno );
      fprintf(stderr,
	      "Getting Shared memory was failed..... (shm_key : %ld, errno : %s)\n",
	      shm_meta.shm_key, strerror(errno));
      return -1;
    }
  }

  /* Attaching shared memory. */
  shm_addr = shmat(shm_id, (void*)0, 0);
  if ( shm_addr == (void*)(-1) )
  {
    fprintf(stderr,
  	    "Getting sharedaddr : %p\n", shm_meta.shm_start_addr);
    printf("cur_addr : %p\n", cur_addr);
  }

  if (shmctl(shm_id, IPC_STAT, &shm_ds) == -1){

    printf( "[Error] Getting shred info failed...\n");
    return -1;
  }

  /* printf( "[Info] loading shm_addr : %p\n", shm_ds.shm_perm */
  printf( "[Info] shm_ds.shm_segsz : %ld\n", shm_ds.shm_segsz);
  printf( "[Info] shm_ds.shm_ctime : %ld\n", shm_ds.shm_ctime);


  /* shm_exists = 0; */
  if ( shm_exists ){

    /* Loading the shared memory meta information. */
    printf( "[Info] loading shm_addr : %p\n", shm_addr);
    for(i = 0; i<100; i++){
      printf("%p", (shm_addr));
    }

    /* wv_shm_load_meta(shm_addr); */
  }
  else{

    for(i = 0; i<100; i++){
      printf("%p", (shm_addr));
    }

    /* Setting the shared memory meta information. */
    printf( "[Info] initializing shm_addr : %p\n", shm_addr);

    /* wv_shm_init_meta(shm_addr); */
  }

  return ret;
}


int wv_shm_test(){
  int ret = 0;
  wv_shm_init();

  return ret;
}


int wv_shm_load_meta(void* shm_start_addr){
  int ret = 0;

  shm_meta = *((wv_shm_meta_t*)wv_shm_rd(shm_start_addr, sizeof(shm_meta), NULL));
  fprintf(stdout, "< The loaded shared meta data >\n");
  printf("* shm_meta.count : %ld\n", shm_meta.count);
  printf("* shm_meta.shm_start_addr : %p\n", shm_meta.shm_start_addr);
  printf("* shm_meta.shm_end_addr : %p\n", shm_meta.shm_end_addr);
  printf("* shm_meta.key : %ld\n", shm_meta.shm_key);
  printf("* shm_meta.shm_junk_size : %ld\n", shm_meta.shm_junk_size);
  printf("* shm_meta.shm_total_size : %ld\n", shm_meta.shm_total_size);

  return ret;
}

int wv_shm_init_meta(void* shm_start_addr){

  int ret = 0;
  int i = 0;
  size_t page_size, shm_alloc_size = 0;
  void* cur_addr = NULL;

  shm_meta.shm_start_addr = shm_start_addr;
  page_size = getpagesize();
  shm_alloc_size = (SHM_TOTAL_SIZE / page_size) * page_size + page_size;
  shm_meta.shm_end_addr = shm_meta.shm_start_addr + shm_alloc_size;
  shm_meta.shm_total_size = shm_alloc_size;
  memset(shm_meta.shm_start_addr, 0x00, sizeof(shm_alloc_size));

  /* Split the section of shared memory */
  cur_addr = shm_meta.shm_start_addr;
  shm_meta.count = 0;
  for ( i=0; i < SHM_MAX_COUNT && (cur_addr < shm_meta.shm_end_addr); i++ ){

    wv_shm_junk_init(&shm_meta.arr_shm_junk_hdr[i], "", cur_addr, cur_addr+SHM_JUNK_SIZE);
    cur_addr += SHM_JUNK_SIZE;
    shm_meta.count++;
  }

  return ret;
}


int wv_shm_sync_meta(wv_shm_meta_t* shm_meta)
{
  int ret = 0;

  if ( wv_shm_wr( shm_meta->shm_start_addr,
		  shm_meta,
		  sizeof(wv_shm_meta_t),
		  NULL,
		  shm_meta->shm_end_addr ) == NULL ){
    fprintf(stderr, "When wrote the shared memory meta data to the shared memory, error occured. ");
    return ret = 1;
  }

  return ret;
}


wv_shm_junk_hdr_t* wv_shm_find_junk(wv_shm_meta_t* shm_meta, const char* junk_name)
{
  wv_shm_junk_hdr_t* ret_shm_junk_hdr = NULL;
  int i = 0;

  if ( shm_meta ){
    for(i = 0; i < shm_meta->count; i++){
      ret_shm_junk_hdr = &(shm_meta->arr_shm_junk_hdr[i]);
      if( ret_shm_junk_hdr->is_assigned &&
	  strncmp(ret_shm_junk_hdr->shm_name, junk_name, strlen(junk_name)) == 0 )
	{
	  return ret_shm_junk_hdr;
	}
    }
  }

  return ret_shm_junk_hdr = NULL;
}


wv_shm_junk_hdr_t* wv_shm_assign_junk(wv_shm_meta_t* shm_meta, const char* junk_name)
{
  wv_shm_junk_hdr_t* ret_shm_junk_hdr = NULL;
  int i = 0;

  if ( shm_meta ){

    for ( i = 0; i < shm_meta->count; i++ ){

      ret_shm_junk_hdr = &(shm_meta->arr_shm_junk_hdr[i]);

      if(wv_shm_find_junk(shm_meta, junk_name)){

	fprintf(stderr, "When the shared memory junk assigned, The junk having the same name was existing\n" );
	return ret_shm_junk_hdr = NULL;
      }

      if ( ret_shm_junk_hdr->is_assigned == 0 ){

	ret_shm_junk_hdr->is_assigned = 1;
	snprintf(ret_shm_junk_hdr->shm_name, 256, "%s", junk_name);

	wv_shm_clear_junk(ret_shm_junk_hdr);

	return ret_shm_junk_hdr;
      }
    }
  }
  else{
    fprintf(stderr, "When the shared memory junk assigned, The junk having the same name was existing\n" );
  }

  return ret_shm_junk_hdr = NULL;
}


wv_shm_junk_hdr_t* wv_shm_unassign_junk(wv_shm_meta_t* shm_meta, const char* junk_name)
{
  wv_shm_junk_hdr_t* ret_shm_junk_hdr = NULL;
  int i = 0;

  if ( shm_meta ){

    for ( i = 0; i < shm_meta->count; i++ ){
      ret_shm_junk_hdr = &shm_meta->arr_shm_junk_hdr[i];

      if ( ret_shm_junk_hdr->is_assigned == 1  &&
	   strncmp(ret_shm_junk_hdr->shm_name, junk_name, strlen(junk_name)) == 0 ){

	ret_shm_junk_hdr->is_assigned = 0;
	strcpy(ret_shm_junk_hdr->shm_name, "");

	wv_shm_clear_junk(ret_shm_junk_hdr);

	return ret_shm_junk_hdr;
      }
    }
  }

  return ( ret_shm_junk_hdr = NULL );
}


int wv_shm_clear_junk(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  int ret = 0;
  if ( shm_junk_hdr &&
       shm_junk_hdr->start_addr &&
       shm_junk_hdr->end_addr )
  {
    memset(shm_junk_hdr->start_addr, 0x00 , shm_junk_hdr->start_addr - shm_junk_hdr->end_addr);

    shm_junk_hdr->write_addr = shm_junk_hdr->start_addr;
    shm_junk_hdr->read_addr = shm_junk_hdr->start_addr;
    shm_junk_hdr->prev_write_addr = NULL;
    shm_junk_hdr->remain_size = shm_junk_hdr->start_addr - shm_junk_hdr->end_addr;
    shm_junk_hdr->count = 0;
  }
  else{
    ret = 1;
    fprintf(stderr, "When cleared the shared memory junk, we received a null value.\n");
  }

  return ret;
}


int wv_shm_dump_junk(wv_shm_meta_t* shm_meta, const char* junk_name,
		 const char* dir_name, const char* file_name)
{
  int ret = 0;
  return ret;
}




int wv_shm_junk_init(wv_shm_junk_hdr_t* shm_junk_hdr, char* shm_junk_name, void* start_addr, void* end_addr)
{
  int ret = 0;
  void* junk_start_addr = NULL;

  int available_size = end_addr - start_addr;

  memset(shm_junk_hdr, 0x00, sizeof(wv_shm_junk_hdr_t));

  if ( available_size < sizeof(wv_shm_junk_hdr_t) )
  {
    ret = -1;
    fprintf(stderr, "When write shared memory junk header to shared memory,\
 the structure was bigger than the available size...\n");
    goto wv_shm_junk_init_ret;
  }

  wv_shm_wr(start_addr, shm_junk_hdr, sizeof(wv_shm_junk_hdr_t), &junk_start_addr, end_addr);

  shm_junk_hdr->start_addr = junk_start_addr;
  shm_junk_hdr->end_addr = end_addr;
  shm_junk_hdr->write_addr = shm_junk_hdr->read_addr = junk_start_addr;
  shm_junk_hdr->prev_write_addr = NULL;
  shm_junk_hdr->remain_size = end_addr - junk_start_addr;
  shm_junk_hdr->count = 0;
  snprintf(shm_junk_hdr->shm_name, 256, "%s", shm_junk_name);

 wv_shm_junk_init_ret:

  return ret;
}



void* wv_shm_wr(void* start_addr, void* data, size_t size, void** next_addr, void* limit_addr)
{
  void* alloc_addr = NULL;

  if (start_addr && limit_addr)
  {
    if( start_addr > limit_addr )
    {
      fprintf(stdout, "start address was higher than end address..\n");
      goto wv_shm_wr_elem_ret;
    }

    if( (start_addr + size) > limit_addr){

      fprintf(stdout, "The shared memory address after writing was higher than limit_addr.\n");
      goto wv_shm_wr_elem_ret;
    }


    if ((alloc_addr = memcpy(start_addr, data, size)))
    {
      *next_addr = alloc_addr + size;
    }
    else
    {
      fprintf(stderr, "memset() was failed.... (errono : %s)\n", strerror(errno));
    }
  }

 wv_shm_wr_elem_ret:

  return alloc_addr;
}


void* wv_shm_wr_elem(wv_shm_junk_hdr_t* shm_junk_hdr, void* data, size_t size)
{
  void* alloc_addr = NULL;
  wv_shm_junk_elem_hdr_t shm_junk_elem_hdr;

  if (shm_junk_hdr &&
      shm_junk_hdr->write_addr &&
      shm_junk_hdr->start_addr &&
      shm_junk_hdr->end_addr)
  {
    if ( shm_junk_hdr->write_addr < shm_junk_hdr->start_addr )
    {
      fprintf(stdout, "write area was lower than end address..\n");
      goto wv_shm_wr_elem_ret;
    }

    if ( (shm_junk_hdr->write_addr + size) > shm_junk_hdr->end_addr )
    {
      fprintf(stdout, "write area was higher than end address..\n");
      goto wv_shm_wr_elem_ret;
    }

    /* Check the write address exceed junk_memory or not and make it coreccted. */

    /* +-------------------------+        +-------------------------+ */
    /* |                   elem_data  =>  |elem_data                | */
    /* +-------------------------+        +-------------------------+ */

    if ( ( shm_junk_hdr->write_addr + sizeof(wv_shm_junk_elem_hdr_t) + size ) > shm_junk_hdr->end_addr ){
      shm_junk_hdr->write_addr = shm_junk_hdr->start_addr;
    }

    /* Links the two elements if the prev_write_addr exists. */
    if ( shm_junk_hdr->prev_write_addr ){
      wv_shm_link_elems(shm_junk_hdr->prev_write_addr, shm_junk_hdr->write_addr);
    }

    shm_junk_hdr->prev_write_addr = shm_junk_hdr->write_addr;



    /* Write the element header information */
    memset(&shm_junk_elem_hdr, 0x00, sizeof(shm_junk_elem_hdr));
    shm_junk_elem_hdr.size = size;
    shm_junk_elem_hdr.next_addr = NULL;

    if ((alloc_addr = memcpy(shm_junk_hdr->write_addr,
			     (void*)&shm_junk_elem_hdr,
			     sizeof(wv_shm_junk_elem_hdr_t))))
    {
      if ( alloc_addr != shm_junk_hdr->write_addr )
      {
	alloc_addr = NULL;
	fprintf(stderr,
		"[Error] The return value of malloc() and head->write_addr was not equal..\
                 (alloc_addr : %p <--> write_addr : %p)\n",
		alloc_addr,
		shm_junk_hdr->write_addr);

	goto wv_shm_wr_elem_ret;
      }

      shm_junk_hdr->write_addr += sizeof(wv_shm_junk_elem_hdr_t);
    }
    else
    {
	fprintf(stdout, "[Error] memcpy() was failed when write a elem_header.");
	goto wv_shm_wr_elem_ret;
    }

    /* Write the element data information */
    if ((alloc_addr = memcpy(shm_junk_hdr->write_addr, data, size)))
    {
      // debug message
      if ( alloc_addr != shm_junk_hdr->write_addr )
      {
	alloc_addr = NULL;
	fprintf(stderr,
		"[Error] The return value of malloc() and head->write_addr was not equal..\
                 (alloc_addr : %p <--> write_addr : %p)\n",
		alloc_addr,
		shm_junk_hdr->write_addr);

	goto wv_shm_wr_elem_ret;
      }


      fprintf(stdout, "Writing pos : %p..\n", alloc_addr);

      shm_junk_hdr->write_addr = shm_junk_hdr->write_addr + size;
      shm_junk_hdr->count++;
    }
    else
    {
	fprintf(stdout, "[Error] memcpy() was failed when write a elem_data.");
	goto wv_shm_wr_elem_ret;
    }
  }

 wv_shm_wr_elem_ret:

  return alloc_addr;
}


/* Links the two elements */
int wv_shm_link_elems(void* prev_elem_addr, void* cur_elem_addr)
{
  int ret = 0;

  wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr = NULL;

  if (prev_elem_addr && cur_elem_addr){
    shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)prev_elem_addr;
    shm_junk_elem_hdr->next_addr = cur_elem_addr;
  }
  else{
    ret = -1;
  }

  return ret;
}


void* wv_shm_rd(void* start_addr, size_t size, void** next_addr)
{
  void* ret = NULL;

  if ( start_addr ){

    *next_addr = start_addr + size;
    ret = start_addr;
  }

  return ret;
}


/* Return the read addres and add a size value to the read address */
void* wv_shm_peek_elem(wv_shm_junk_hdr_t* shm_junk_hdr, size_t size)
{
  void* ret = NULL;
  if (shm_junk_hdr &&
      shm_junk_hdr->start_addr &&
      shm_junk_hdr->end_addr &&
      shm_junk_hdr->read_addr)
  {
    fprintf(stdout, "Reading pos : %p..\n", shm_junk_hdr->read_addr);
    ret = shm_junk_hdr->read_addr;
    shm_junk_hdr->read_addr += size;
  } 
  else{
    fprintf(stderr, "shm_hdr is null");
  }

  size_t *test_int = (size_t*)ret;
  fprintf(stderr, "Address of ret : %p\n", ret);
  fprintf(stderr, "Readed shm_hdr int value : %ld\n", *test_int);

  return ret;
}


/* Read a element from shared memory and memeset() with 0x00 */
int wv_shm_del_last_elem(wv_shm_junk_hdr_t* shm_junk_hdr){
  int ret = 0;
  wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr = NULL;

  if(shm_junk_hdr &&
     shm_junk_hdr->start_addr &&
     shm_junk_hdr->end_addr &&
     shm_junk_hdr->read_addr)
  {
    shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)wv_shm_peek_elem(shm_junk_hdr, sizeof(wv_shm_junk_elem_hdr_t));

    if (shm_junk_elem_hdr->next_addr){
      shm_junk_hdr->read_addr = shm_junk_elem_hdr->next_addr;
    }
    else{
      ret = -1;
      fprintf(stdout, "When delete a last element, the next_addr of the element was NULL.\n");
      goto wv_shm_rm_elem_ret;
    }
  }
  else{
    ret = -1;
    fprintf(stderr, "When delete a last element, shm_junk_hdr is NULL\n");
    goto wv_shm_rm_elem_ret;
  }

 wv_shm_rm_elem_ret:

  return ret;
}



/* int wv_shm_test () */
/* { */
/*   int ret = 0; */
/*   void* shm_addr = (void*)0; */
/*   void* cur_addr = NULL; */

/*   int i, j, shm_alloc_size, page_size  = 0; */
/*   int shm_id = 0; */

/*   struct shmid_ds shm_info; */
    
/*   fprintf(stdout, "sizeof size_t : %ld\n", sizeof(size_t)); */

/*   memset(&shm_meta, 0x00, sizeof(shm_meta)); */
/*   shm_meta.count = 0; */
/*   shm_meta.shm_key = SHM_KEY; */
/*   shm_meta.shm_junk_size = SHM_JUNK_SIZE; */

/*   /\* Remove previous shared memory *\/ */
/*   if ((shm_id = shmget(shm_meta.shm_key, 0, 0)) != -1){ */
/*     fprintf(stderr, */
/* 	    "[Success] Shared memory was existing.... (shm_id : %d, errno : %s)\n", */
/* 	    shm_id, strerror(errno)); */
/*     shmctl(shm_id, IPC_RMID, 0); */
/*   } */
  
/*   /\* Getting shared memory *\/ */
/*   if ((shm_id = shmget(shm_meta.shm_key, */
/* 		       SHM_TOTAL_SIZE, */
/*   		       IPC_CREAT|0777)) == -1) */
/*   { */
/*     perror( "shmget" ); */
/*     exit( errno ); */
/*     fprintf(stderr, */
/*   	    "Getting Shared memory was failed..... (shm_key : %ld, errno : %s)\n", */
/*   	    shm_meta.shm_key, strerror(errno)); */
/*     ret = -1; */
/*     goto shm_test_ret; */
/*   } */


/*   /\* Attaching shared memory. *\/ */
/*   shm_addr = shmat(shm_id, (void*)0, 0); */
/*   if ( shm_addr == (void*)(-1) ) */
/*   { */
/*     fprintf(stderr, */
/*   	    "Getting shared memory address was failed.... (shm_id : %d, errno : %s)\n", */
/*   	    shm_id, strerror(errno)); */
/*     ret = -1; */
/*     goto shm_test_ret; */
/*   } */

/*   /\* Setting shared memory meta file. *\/ */
/*   shm_meta.shm_start_addr = shm_addr; */
/*   page_size = getpagesize(); */
/*   shm_alloc_size = (page_size) * page_size + page_size; */
/*   shm_meta.shm_end_addr = shm_meta.shm_start_addr + shm_alloc_size; */
/*   shm_meta.shm_total_size = shm_alloc_size; */
/*   memset(shm_meta.shm_start_addr, 0x00, sizeof(shm_alloc_size)); */

/*   printf("< Shared memory information >\n"); */
/*   printf("The size of OS page : %d\n", getpagesize()); */
/*   printf("Shared memory start : %p\n", shm_meta.shm_start_addr); */
/*   printf("Shared memory end : %p\n", shm_meta.shm_end_addr); */
/*   printf("Shared memory total size : %ld\n", shm_meta.shm_total_size); */
/*   puts(""); */
/*   puts(""); */
    

/*   if (shmctl(shm_id, IPC_STAT, &shm_info) == -1){ */
/*     printf( "[Error] Getting shred info failed...\n"); */
/*     return -1; */
/*   } */

/*   cur_addr = shm_meta.shm_start_addr; */
/*   if(wv_shm_wr(cur_addr, &shm_meta, sizeof(wv_shm_meta_t), &cur_addr, shm_meta.shm_end_addr)){ */
/*     printf("shm_meta start addr : %p\n", shm_meta.shm_start_addr); */
/*     printf("cur_addr : %p\n", cur_addr); */
/*   } */

/*   wv_shm_meta_t* shm_meta_test = (wv_shm_meta_t*)shm_meta.shm_start_addr; */
/*   printf("shm_meta_test->shm_start_addr : %p\n", shm_meta_test->shm_start_addr); */
/*   printf("shm_meta_test->shm_end_addr : %p\n", shm_meta_test->shm_end_addr); */
/*   printf("shm_meta_test->shm_total_size : %ld\n", shm_meta_test->shm_total_size); */

/*   /\* Split the section of shared memory *\/ */
/*   for (i=0; i<SHM_MAX_COUNT && (cur_addr < shm_meta.shm_end_addr); i++){ */
/*     printf("init junk index : %d\n", i); */
/*     wv_shm_junk_init(&shm_meta.arr_shm_junk_hdr[i], "", cur_addr, cur_addr+SHM_JUNK_SIZE); */
/*     cur_addr += SHM_JUNK_SIZE; */
/*   } */

/*   wv_shm_junk_hdr_t* shm_junk_hdr = &(shm_meta.arr_shm_junk_hdr[0]); */

/*   printf("shm_junk_info->end_addr : %p\n", shm_junk_hdr->end_addr); */
/*   cur_addr = shm_junk_hdr->start_addr; */

/*   /\* Write data to shared memory *\/ */
/*   printf("<============== Start Writing ==============>\n"); */

/*   /\* Make a element *\/ */
/*   for(i=0; i<3; i++){ */

/*     wv_shm_junk_elem_hdr_t shm_junk_elem; */
/*     shm_junk_elem.attr_count = 0; */
/*     memset(&shm_junk_elem, 0x00, sizeof(wv_shm_junk_elem_hdr_t)); */

/*     for(j=0; j<3; j++){ */
/*       wv_shm_junk_elem_attr_t* attr = &shm_junk_elem.attrs[j]; */
/*       memset(attr, 0x00, sizeof(wv_shm_junk_elem_attr_t)); */

/*       /\* Write attributes key *\/ */
/*       attr->key = (void*)malloc(sizeof(5)); */
/*       memset(attr->key, 0x00, 5); */
/*       snprintf(attr->key, 5, "%s%i", "key", j); */
/*       printf("attr.key : %s\n",attr->key); */
/*       attr->key_size = strlen(attr->key) + 1; */

/*       /\* Write attributes data *\/ */
/*       int data_size = strlen("testtest") + 1; */
/*       attr->data = (void*)malloc(data_size); */
/*       memset(attr->data, 0x00, data_size); */
/*       snprintf(attr->data, strlen("testtest")+1, "%s", "testtest"); */
/*       attr->attr_size = data_size; */

/*       /\* Write attributes data size *\/ */
/*       attr->attr_size = data_size; */

/*       shm_junk_elem.attr_count++; */
/*       shm_junk_elem.total_size += (sizeof(size_t) + sizeof(size_t) + attr->key_size + attr->attr_size); */
/*     } */

/*     /\* Write the element header information to memory *\/ */

/*     wv_shm_wr_elem(shm_junk_hdr, (void*)&(shm_junk_elem.total_size), sizeof(size_t)); */
/*     printf("shm_junk_lem.total_size : %ld\n", shm_junk_elem.total_size); */
/*     wv_shm_wr_elem(shm_junk_hdr, (void*)&(shm_junk_elem.attr_count), sizeof(size_t)); */
/*     printf("shm_junk_lem.attr_count : %ld\n", shm_junk_elem.attr_count); */

/*     /\* Write the attributes information of a element to memory *\/ */
/*     printf("Writing attributes.\n"); */

/*     for(j=0; j<3; j++){ */

/*       wv_shm_junk_elem_attr_t* attr = &shm_junk_elem.attrs[j]; */
/*       printf("Writing key_size : %ld.\n", attr->key_size); */
/*       wv_shm_wr_elem(shm_junk_hdr, (void*)&attr->key_size, sizeof(size_t)); */
/*       printf("Writing data_size : %ld.\n", attr->attr_size); */
/*       wv_shm_wr_elem(shm_junk_hdr, (void*)&attr->attr_size, sizeof(size_t)); */
/*       printf("Writing key : %s\n", attr->key); */
/*       wv_shm_wr_elem(shm_junk_hdr, (void*)attr->key, attr->key_size); */
/*       printf("Writing data : %s\n", attr->data); */
/*       wv_shm_wr_elem(shm_junk_hdr, (void*)attr->data, attr->attr_size); */
/*     } */
/*   } */

/*   /\* Read data to shared memory *\/ */
/*   printf("<============== Start Reading ==============>\n"); */
/*   for(i=0; i<shm_junk_hdr->count; i++){ */
/*     wv_shm_junk_elem_hdr_t shm_junk_elem_hdr; */
/*     memset(&shm_junk_elem_hdr, 0x00, sizeof(wv_shm_junk_elem_hdr_t)); */
/*     cur_addr = shm_junk_hdr->start_addr; */

/*     /\* Read total size *\/ */
/*     shm_junk_elem_hdr.total_size = *((size_t*)wv_shm_rd_elem(shm_junk_hdr, sizeof(size_t))); */

/*     /\* Read attrs count  *\/ */
/*     shm_junk_elem_hdr.attr_count = *((size_t*)wv_shm_rd_elem(shm_junk_hdr, sizeof(size_t))); */
 
/*     printf("(SHM_INIT) shm_elem_hdr_total_size : %ld\n", shm_junk_elem_hdr.total_size); */
/*     printf("(SHM_INIT) shm_elem_hdr_count : %ld\n", shm_junk_elem_hdr.attr_count); */

/*     for (j=0; j<shm_junk_elem_hdr.attr_count; j++){ */
/*       size_t key_size = 0; */
/*       size_t data_size = 0; */
/*       char* shm_elem_attr_key = NULL; */
/*       char* shm_elem_attr_data = NULL; */

/*       key_size = *((size_t*)wv_shm_peek_elem(shm_junk_hdr, sizeof(size_t))); */
/*       data_size = *((size_t*)wv_shm_peek_elem(shm_junk_hdr, sizeof(size_t))); */
/*       shm_elem_attr_key = (char*)wv_shm_peek_elem(shm_junk_hdr, key_size); */
/*       shm_elem_attr_data = (char*)wv_shm_peek_elem(shm_junk_hdr, data_size); */

/*       printf("key_size : %ld\n", key_size); */
/*       printf("data_size : %ld\n", data_size); */
/*       printf("attr_key : %s\n", shm_elem_attr_key); */
/*       printf("attr_data : %s\n", shm_elem_attr_data); */
/*     } */
/*   } */

/*  shm_test_ret: */

/*   return ret; */
/* } */


