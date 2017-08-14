#include "wv_shm_mngt.h"


#define SHMMAX 5000
#define SHMMIN 1000

typedef struct wv_packet{
  int packet_id;
  char data[8192];
}wv_packet_t;

wv_shm_meta_t* shm_meta = NULL;


int wv_shm_init ()
{

  fprintf(stdout, "[ %s ]\n" , __func__);

  int ret = 0;
  void* shm_addr = (void*)0;
  void* cur_addr = NULL;

  int shm_id = 0;
  int shm_exists = 0;

  struct shmid_ds shm_ds;

  shm_meta = (wv_shm_meta_t*)malloc(sizeof(wv_shm_meta_t));
  memset(shm_meta, 0x00, sizeof(wv_shm_meta_t));

  shm_meta->count = 0;
  shm_meta->shm_key = SHM_KEY;
  shm_meta->shm_junk_size = SHM_JUNK_SIZE;
  shm_meta->shm_total_size = SHM_TOTAL_SIZE;

  

  /* Getting shared memory */
  if ( (shm_id = shmget(shm_meta->shm_key, 0, 0)) != -1 ){

    shm_exists = 1;
    wv_write_log(LOG_INF,
		 "Previous shared memory was existing.... (shm_id : %d, errno : %s)",
		 shm_id, strerror(errno));
    shmctl(shm_id, IPC_RMID, 0);
  }
  else{
    wv_write_log(LOG_INF, "shm_meta->shm_key : %ld", shm_meta->shm_key);
    wv_write_log(LOG_INF, "shm_meta->shm_total_size : %ld", shm_meta->shm_total_size);

    if ( (shm_id = shmget(shm_meta->shm_key,
			  shm_meta->shm_total_size,
			  IPC_CREAT|0777)) == -1 ){

      fprintf(stderr,
	      "Getting Shared memory was failed..... (shm_key : %ld, errno : %s)\n",
	      shm_meta->shm_key, strerror(errno));

      exit( errno );
      return -1;
    }
  }

  /* Attaching shared memory. */
  shm_addr = shmat(shm_id, (void*)0, 0);
  if ( shm_addr == (void*)(-1) )
  {
    fprintf(stderr,
  	    "Getting sharedaddr : %p\n", shm_meta->shm_start_addr);
    printf("cur_addr : %p\n", cur_addr);
  }

  if (shmctl(shm_id, IPC_STAT, &shm_ds) == -1){

    printf( "[Error] Getting shred info failed...\n");
    return -1;
  }

  wv_write_log(LOG_INF, "shm_ds.shm_segsz : %ld", shm_ds.shm_segsz);
  wv_write_log(LOG_INF, "shm_ds.shm_ctime : %ld", shm_ds.shm_ctime);

  shm_exists = 0;
  if ( shm_exists ){

    wv_shm_load_meta(shm_addr);
  }
  else{
    /* Setting the shared memory meta information. */
       
    wv_shm_init_meta(shm_addr);
    wv_shm_sync_meta(shm_meta);
  }

  return ret;
}


int wv_shm_load_meta(void* shm_start_addr){
  int ret = 0;

  shm_meta = (wv_shm_meta_t*)wv_shm_rd(shm_start_addr, sizeof(wv_shm_meta_t), NULL);

  fprintf(stdout, "< The loaded shared meta data >\n");
  printf("* shm_meta.count : %ld\n", shm_meta->count);
  printf("* shm_meta.shm_start_addr : %p\n", shm_start_addr);
  printf("* shm_meta.shm_end_addr : %p\n", shm_start_addr + shm_meta->shm_total_size);
  printf("* shm_meta.key : %ld\n", shm_meta->shm_key);
  printf("* shm_meta.shm_junk_size : %ld\n", shm_meta->shm_junk_size);
  printf("* shm_meta.shm_total_size : %ld\n", shm_meta->shm_total_size);

  return ret;
}

int wv_shm_init_meta(void* shm_start_addr)
{

  wv_write_log(LOG_INF, "[ %s ]", __func__);

  int ret = 0;
  int i = 0;
  size_t page_size, shm_alloc_size = 0;
  size_t cur_offs = 0;

  shm_meta->shm_start_addr = shm_start_addr;
  page_size = getpagesize();
  shm_alloc_size = (SHM_TOTAL_SIZE / page_size) * page_size + page_size;
  shm_meta->shm_end_addr = shm_meta->shm_start_addr + shm_alloc_size;
  shm_meta->shm_total_size = shm_alloc_size;
  memset(shm_meta->shm_start_addr, 0x00, sizeof(shm_alloc_size));

  wv_write_log(LOG_INF, "Initailizing the shared memory meta structure...");
  wv_write_log(LOG_INF, "shm_meta->count : %ld", shm_meta->count);
  wv_write_log(LOG_INF, "shm_meta->shm_start_addr : %p", shm_meta->shm_start_addr);
  wv_write_log(LOG_INF, "shm_meta->shm_end_addr : %p", shm_meta->shm_end_addr);
  wv_write_log(LOG_INF, "shm_meta->key : %ld", shm_meta->shm_key);
  wv_write_log(LOG_INF, "shm_meta->shm_junk_size : %ld", shm_meta->shm_junk_size);
  wv_write_log(LOG_INF, "shm_meta->shm_total_size : %ld", shm_meta->shm_total_size);

  /* Split the section of shared memory */
  cur_offs = sizeof(wv_shm_meta_t);
  shm_meta->count = 0;

  wv_write_log(LOG_INF, "Initailizing the shared memory junk queuere...");
  for ( i=0; i < SHM_MAX_COUNT && ((shm_meta->shm_start_addr + cur_offs) < shm_meta->shm_end_addr); i++ ){

    wv_write_log(LOG_INF, "Initailizing shared memory junk[%d]", i);
    wv_shm_junk_init(&shm_meta->arr_shm_junk_hdr[i], "", cur_offs, cur_offs + SHM_JUNK_SIZE);
    cur_offs += SHM_JUNK_SIZE;
    shm_meta->count++;
  }

  return ret;
}

int wv_shm_chk_offs(size_t offset){

  int ret = 1;

  if ( shm_meta ){

    if ( (shm_meta->shm_start_addr + offset) > shm_meta->shm_end_addr ){
      ret = -1;
    }
  }

  return ret;
}


int wv_shm_sync_meta()
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  int ret = 0;

  if ( shm_meta && wv_shm_wr( 0,
			      shm_meta,
			      sizeof(wv_shm_meta_t),
			      NULL ) == NULL ){

    wv_write_log(LOG_ERR, "When wrote the shared memory meta data to the shared memory, error occured. ");
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


wv_shm_junk_hdr_t* wv_shm_assign_junk(const char* junk_name)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  wv_shm_junk_hdr_t* ret_shm_junk_hdr = NULL;
  int i = 0;
  if ( shm_meta && junk_name ){

    for ( i = 0; i < shm_meta->count; i++ ){

      ret_shm_junk_hdr = &(shm_meta->arr_shm_junk_hdr[i]);

      if(wv_shm_find_junk(shm_meta, junk_name)){

	wv_write_log(LOG_ERR, "When the shared memory junk assigned, The junk having the same name was existing");
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
    wv_write_log(LOG_ERR, "The Ware Valley Shared Memory was not initailized..." );
  }

  return ret_shm_junk_hdr = NULL;
}


wv_shm_junk_hdr_t* wv_shm_unassign_junk(wv_shm_meta_t* shm_meta, const char* junk_name)
{
  fprintf(stdout, "[ %s ]\n" , __func__);

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



wv_shm_junk_hdr_t** wv_shm_get_junk_list(){

  wv_shm_junk_hdr_t** ret_junk_hdr_list = NULL;
  int i = 0;

  if (shm_meta){

    ret_junk_hdr_list = (wv_shm_junk_hdr_t**)malloc(sizeof(wv_shm_junk_hdr_t*) * (shm_meta->count + 1) + 1);
    memset(ret_junk_hdr_list, 0x00, sizeof(wv_shm_junk_hdr_t*) * (shm_meta->count + 1));

    for(i = 0; i<shm_meta->count+1; i++){

      (*(ret_junk_hdr_list+i)) = (wv_shm_junk_hdr_t*)malloc(sizeof(wv_shm_junk_hdr_t));
      snprintf((*(ret_junk_hdr_list+i))->shm_name, 256, "%s", shm_meta->arr_shm_junk_hdr[i].shm_name);
      (*(ret_junk_hdr_list+i))->count = shm_meta->arr_shm_junk_hdr[i].count;
      (*(ret_junk_hdr_list+i))->is_assigned = shm_meta->arr_shm_junk_hdr[i].is_assigned;
      (*(ret_junk_hdr_list+i))->prev_write_offset = shm_meta->arr_shm_junk_hdr[i].prev_write_offset;
      (*(ret_junk_hdr_list+i))->quu_start_offset = shm_meta->arr_shm_junk_hdr[i].quu_start_offset;
      (*(ret_junk_hdr_list+i))->quu_end_offset = shm_meta->arr_shm_junk_hdr[i].quu_end_offset;
      (*(ret_junk_hdr_list+i))->read_offset = shm_meta->arr_shm_junk_hdr[i].read_offset;
      (*(ret_junk_hdr_list+i))->remain_size = shm_meta->arr_shm_junk_hdr[i].remain_size;
      (*(ret_junk_hdr_list+i))->start_offset = shm_meta->arr_shm_junk_hdr[i].start_offset;
      (*(ret_junk_hdr_list+i))->write_offset = shm_meta->arr_shm_junk_hdr[i].write_offset;
    }
  }
  else{
    fprintf(stderr, "shm_meta was not initialized...");
  }

  return ret_junk_hdr_list;
}


void wv_shm_free_junk_list(wv_shm_junk_hdr_t** junk_hdr_list){

  int i = 0;
  printf("junk_list : %p\n", junk_hdr_list);

  if ( junk_hdr_list ){

    for (i = 0; junk_hdr_list[i]; i++){
      if(junk_hdr_list[i]){
	free(junk_hdr_list[i]);
	printf("loop");
      }
    }
  }

  if (junk_hdr_list){
    free(junk_hdr_list);
  }
}


int wv_shm_clear_junk(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  int ret = 0;

  if(shm_meta){

    if ( shm_junk_hdr &&
	 shm_junk_hdr->quu_start_offset &&
	 shm_junk_hdr->quu_end_offset ){

	memset( (shm_meta->shm_start_addr + shm_junk_hdr->quu_start_offset),
	        0x00,
		(shm_junk_hdr->quu_end_offset - shm_junk_hdr->quu_start_offset) );

	shm_junk_hdr->write_offset = shm_junk_hdr->quu_start_offset;
	shm_junk_hdr->read_offset = shm_junk_hdr->quu_start_offset;
	shm_junk_hdr->prev_write_offset = 0;
	shm_junk_hdr->remain_size =  shm_junk_hdr->quu_end_offset - shm_junk_hdr->quu_start_offset;
	shm_junk_hdr->count = 0;
    }
    else{

      ret = 1;
      fprintf(stderr, "When cleared the shared memory junk, we received a null value.\n");
    }
  }


  return ret;
}


/* int wv_shm_dump_junk(const char* junk_name, const char* dir_name, const char* file_name) */
/* { */
/*   fprintf(stdout, "[ %s ]\n" , __func__); */

/*   int ret = 0; */
/*   int wr_buf_size = 8192; */
/*   char full_path[8192] = {0}; */
/*   void* cur_pos = NULL; */
/*   FILE* pfile = NULL; */
/*   const char* dlmt = "/"; */
/*   wv_shm_junk_hdr_t* shm_junk_hdr = NULL; */
/*   int nwrite = 0; */

/*   snprintf( full_path, 8192, "%s%s%s", dir_name, dlmt, file_name ); */

/*   /\* Find if there is shm_junk existing or not. *\/ */
/*   if ( (shm_junk_hdr = wv_shm_find_junk(shm_meta, junk_name)) == NULL ){ */

/*     fprintf( stdout, "Counldnt find the shared memory junk having same name...\n" ); */
/*     ret = -1; goto wv_shm_dump_junk_ret; */
/*   } */

/*   /\* Find if there is a file having the same name or not. *\/ */
/*   if ( access(full_path, F_OK) == 0 ){ */

/*     fprintf( stdout, "There is a file existing...\n" ); */
/*     ret = -1; goto wv_shm_dump_junk_ret; */
/*   } */
/*   else{ */

/*     /\* Dump the founded shm_junk to a file. *\/ */
/*     if ( (pfile = fopen(full_path, "wb")) == NULL ){ */

/*       fprintf( stdout, "Couldnt open the file with wb mode...\n" ); */
/*       ret = -1; goto wv_shm_dump_junk_ret; */
/*     } */
/*   } */

/*   /\* Dump the founed shm_junk to a file. *\/ */
/*   if ( shm_junk_hdr->start_offset ){ */
/*     cur_pos = shm_junk_hdr->start_offset; */

/*     while( 1 ){ */

/*       if ((cur_pos + wr_buf_size) > shm_junk_hdr->quu_end_offset){ */

/* 	fwrite(cur_pos, sizeof(char), shm_junk_hdr->quu_end_offset - cur_pos, pfile); */
/*       } */
/*       else{ */

/* 	fwrite(cur_pos, sizeof(char), wr_buf_size, pfile); */
/* 	break; */
/*       } */
/*       cur_pos += wr_buf_size; */
/*     } */
/*   } */

/*   if (pfile){ */

/*     fclose(pfile); */
/*   } */


/*  wv_shm_dump_junk_ret: */

/*   return ret; */
/* } */



/* int wv_shm_load_junk(wv_shm_meta_t* shm_meta, const char* dir_name, const char* file_name) */
/* { */
/*   int ret = 0; */
/*   return ret; */
/* } */


int wv_shm_junk_init(wv_shm_junk_hdr_t* shm_junk_hdr, char* shm_junk_name, size_t start_offs, size_t end_offs)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  int ret = 0;

  int available_size = end_offs - start_offs;

  memset(shm_junk_hdr, 0x00, sizeof(wv_shm_junk_hdr_t));

  if ( available_size < sizeof(wv_shm_junk_hdr_t) )
  {
    ret = -1;
    wv_write_log(LOG_ERR, "When write shared memory junk header to shared memory, "
		 "the structure was bigger than the available size...");
    goto wv_shm_junk_init_ret;
  }

  /* Write the junk header information to shared memory */
  shm_junk_hdr->start_offset = start_offs;

  if (wv_shm_wr(start_offs, shm_junk_hdr, sizeof(wv_shm_junk_hdr_t), NULL) == NULL){

    ret = -1;
    wv_write_log(LOG_ERR, "Writing the shared memory junk was failed...");
    goto wv_shm_junk_init_ret;
  }

  shm_junk_hdr->quu_start_offset = start_offs;
  shm_junk_hdr->quu_end_offset = end_offs;
  shm_junk_hdr->write_offset = shm_junk_hdr->read_offset = start_offs;
  shm_junk_hdr->prev_write_offset = 0;
  shm_junk_hdr->remain_size = end_offs - start_offs;
  shm_junk_hdr->count = 0;

  wv_write_log(LOG_INF, "shm_junk_hdr->shm_name : %s", shm_junk_hdr->shm_name);
  wv_write_log(LOG_INF, "shm_junk_hdr->is_assigned : %d", shm_junk_hdr->is_assigned);
  wv_write_log(LOG_INF, "shm_junk_hdr->start_offset : %ld", shm_junk_hdr->start_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->start_offset : %ld", shm_junk_hdr->start_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->quu_start_offset : %ld", shm_junk_hdr->quu_start_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->quu_end_offset : %ld", shm_junk_hdr->quu_end_offset );
  wv_write_log(LOG_INF, "shm_junk_hdr->write_offset : %ld", shm_junk_hdr->write_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->prev_write_offset : %ld", shm_junk_hdr->prev_write_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->remain_size : %ld", shm_junk_hdr->remain_size);
  wv_write_log(LOG_INF, "shm_junk_hdr->count : %ld", shm_junk_hdr->count);

 wv_shm_junk_init_ret:

  return ret;
}


void* wv_shm_wr(size_t start_offs, void* data, size_t size, size_t *next_offs)
{
  void* write_addr = NULL;
  void* ret_memcpy = NULL;

  wv_write_log(LOG_INF, "[ %s ]", __func__);

  if ( shm_meta && shm_meta->shm_start_addr ){

    write_addr = shm_meta->shm_start_addr + start_offs;
    if ( write_addr > shm_meta->shm_end_addr ){

      wv_write_log(LOG_ERR, "The address to write was higher than end address.");
      write_addr = NULL;
      goto wv_shm_wr_elem_ret;
    }

    if ( write_addr + size > shm_meta->shm_end_addr ){

      wv_write_log(LOG_ERR, "The address after write was higher than end address.");
      write_addr = NULL;
      goto wv_shm_wr_elem_ret;
    }

    if ( (ret_memcpy = memcpy(write_addr, data, size)) ) {

      if ( write_addr != ret_memcpy ) {
	wv_write_log(LOG_WRN, "The address returned by memcpy was not equaled with write_addr.");
      }

      if (next_offs) {
	*next_offs = start_offs + size;
      }
    }
    else{
      fprintf(stderr, "memset() was failed.... (errono : %s)\n", strerror(errno));
    } 
  }

 wv_shm_wr_elem_ret:

  return write_addr;
}


void* wv_shm_push_elem(wv_shm_junk_hdr_t* shm_junk_hdr, void* data, size_t size)
{
  void* alloc_addr = NULL;
  wv_shm_junk_elem_hdr_t elem_hdr;
  void* write_addr = NULL;
  size_t offs_after_write = 0;

  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  if (shm_junk_hdr &&
      shm_junk_hdr->write_offset &&
      shm_junk_hdr->quu_start_offset &&
      shm_junk_hdr->quu_end_offset)
  {
    if ( shm_junk_hdr->write_offset < shm_junk_hdr->quu_start_offset ){

      wv_write_log(LOG_ERR, "write area was lower than end address..");
      goto wv_shm_wr_elem_ret;
    }

    if ( (shm_junk_hdr->write_offset + size) > shm_junk_hdr->quu_end_offset ){

      wv_write_log(LOG_ERR, "write area was higher than end address..");
      goto wv_shm_wr_elem_ret;
    }

    /* Check the write address exceed junk_memory or not and make it coreccted. */
    /* +-------------------------+        +-------------------------+ */
    /* |                   elem_data  =>  |elem_data                | */
    /* +-------------------------+        +-------------------------+ */
    offs_after_write = shm_junk_hdr->write_offset + sizeof(wv_shm_junk_elem_hdr_t) + size;
    if ( offs_after_write > shm_junk_hdr->quu_end_offset ){

      shm_junk_hdr->write_offset = shm_junk_hdr->quu_start_offset;
    }

    /* Links the two elements if the prev_write_addr exists. */
    wv_shm_junk_elem_hdr_t* test_hdr = NULL;

    if ( shm_junk_hdr->prev_write_offset ){

      /* test_hdr = shm_meta->shm_start_addr + shm_junk_hdr->prev_write_offset; */
      /* wv_write_log(LOG_INF, "test_hdr->size:%ld", test_hdr->size); */
      /* wv_write_log(LOG_INF, "test_hdr->next_offset:%ld", test_hdr->next_offset); */

      wv_shm_link_elems(shm_junk_hdr,
      			shm_junk_hdr->prev_write_offset,
      			shm_junk_hdr->write_offset);

      /* wv_write_log(LOG_INF, "test_hdr->size:%ld", test_hdr->size); */
      /* wv_write_log(LOG_INF, "test_hdr->next_offset:%ld", test_hdr->next_offset); */
    }

    test_hdr = shm_meta->shm_start_addr + shm_junk_hdr->prev_write_offset;
    if(test_hdr){
      wv_write_log(LOG_INF, "test_hdr->size:%ld", test_hdr->size);
      wv_write_log(LOG_INF, "test_hdr->next_offset:%ld", test_hdr->next_offset);
    }


    shm_junk_hdr->prev_write_offset = shm_junk_hdr->write_offset;

    /* Write the element header information */
    memset(&elem_hdr, 0x00, sizeof(wv_shm_junk_elem_hdr_t));
    elem_hdr.size = size;
    elem_hdr.next_offset = 0;
    wv_write_log(LOG_INF, "elem_hdr.size : %ld", elem_hdr.size);
    wv_write_log(LOG_INF, "elem_hdr.next_offset : %ld", elem_hdr.next_offset);

    write_addr = shm_meta->shm_start_addr + shm_junk_hdr->write_offset;
    wv_write_log(LOG_INF, "write position : %p", write_addr);
    if ((alloc_addr = memcpy(write_addr,
			     &elem_hdr,
			     sizeof(wv_shm_junk_elem_hdr_t))))
    {
      if ( alloc_addr != shm_meta->shm_start_addr + shm_junk_hdr->write_offset ){

	alloc_addr = NULL;
	wv_write_log(LOG_ERR,
		     "The returned value of malloc() and head->write_addr was not equal.."
		     "(alloc_addr : %p <--> write_addr : %p)",
		     alloc_addr, write_addr);

	goto wv_shm_wr_elem_ret;
      }

      /* Move the write position of junk_header to the postion added header size. */
      wv_write_log(LOG_INF, "pos_write_hdr : %p (write_offset : %ld)",
		   write_addr, shm_junk_hdr->write_offset);

      shm_junk_hdr->write_offset += sizeof(wv_shm_junk_elem_hdr_t);
    }
    else
    {
	wv_write_log(LOG_ERR, "memcpy() was failed when write a elem_header.");
	goto wv_shm_wr_elem_ret;
    }

    /* Write the element data information */

    write_addr = shm_meta->shm_start_addr + shm_junk_hdr->write_offset;
    if ((alloc_addr = memcpy(write_addr, data, size)))
    {
      // debug message
      if ( alloc_addr != write_addr )
      {
	alloc_addr = NULL;
	wv_write_log(LOG_ERR,
		"The returned value of malloc() and head->write_addr was not equal..\
                 (alloc_addr : %p <--> write_addr : %p)",
		alloc_addr, write_addr);

	goto wv_shm_wr_elem_ret;
      }

      wv_write_log(LOG_INF, "pos_write_data : %p (write_offset : %ld)",
		   write_addr, shm_junk_hdr->write_offset);

      shm_junk_hdr->write_offset = shm_junk_hdr->write_offset + size;
      shm_junk_hdr->count++;
    }
    else
    {
	wv_write_log(LOG_ERR, "memcpy() was failed when write a elem_data.");
	goto wv_shm_wr_elem_ret;
    }
  }

 wv_shm_wr_elem_ret:

  return alloc_addr;
}


/* Links the two elements */
int wv_shm_link_elems(wv_shm_junk_hdr_t* junk_hdr, size_t prev_offset, size_t cur_offset)
{
  int ret = 0;
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr = NULL;

  if (prev_offset && cur_offset){

    shm_junk_elem_hdr = shm_meta->shm_start_addr + prev_offset;
    shm_junk_elem_hdr->next_offset = cur_offset;
    wv_write_log(LOG_INF, "shm_junk_elem_hdr->next_offset : %ld" , shm_junk_elem_hdr->next_offset);
  }
  else{
    ret = -1;
  }

  return ret;
}


void* wv_shm_rd(void* start_addr, size_t size, void** next_addr){

  void* ret = NULL;

  if ( start_addr ){

    if ( next_addr ) { *next_addr = start_addr + size; }
    
    ret = start_addr;
  }

  return ret;
}


wv_shm_junk_elem_hdr_t* wv_shm_peek_elem_hdr(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  wv_write_log(LOG_INF, "[ %s ]", __func__);

  wv_shm_junk_elem_hdr_t* ret_elem_hdr = NULL;
  void* addr_read = NULL;
  size_t offset_read = 0;

  if (shm_junk_hdr &&
      shm_junk_hdr->quu_start_offset &&
      shm_junk_hdr->quu_end_offset &&
      shm_junk_hdr->read_offset){

    offset_read = shm_junk_hdr->read_offset;
    addr_read = shm_meta->shm_start_addr + offset_read;

    wv_write_log(LOG_INF, "offset to read : %ld", offset_read);
    wv_write_log(LOG_INF, "address to read :  %p", addr_read);

    ret_elem_hdr = (wv_shm_junk_elem_hdr_t*)addr_read;
    wv_write_log(LOG_INF, "ret_elem_hdr->size : %ld", ret_elem_hdr->size);
    wv_write_log(LOG_INF, "ret_elem_hdr->next_offset : %ld", ret_elem_hdr->next_offset);
  }
  else{
    wv_write_log(LOG_ERR, "A shm_hdr or its attrs is null...");
  }

  return ret_elem_hdr;
}


/* Return the read addres and add a size value to the read address */
void* wv_shm_peek_elem_data(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  wv_write_log(LOG_INF, "[ %s ]", __func__);

  wv_shm_junk_elem_hdr_t* ret_shm_junk_elem_hdr = NULL;
  void* addr_read = NULL;
  size_t offset_read = 0;

  if ( shm_junk_hdr &&
       shm_junk_hdr->quu_start_offset &&
       shm_junk_hdr->quu_end_offset &&
       shm_junk_hdr->read_offset )
  {

    offset_read = shm_junk_hdr->read_offset + sizeof(wv_shm_junk_elem_hdr_t);
    addr_read = shm_meta->shm_start_addr + offset_read;

    wv_write_log(LOG_INF, "offset to read : %ld", offset_read);
    wv_write_log(LOG_INF, "address to read : %p", addr_read);

    ret_shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)addr_read;
  }
  else{

    wv_write_log(LOG_ERR, "A shm_hdr or shm_junk_elem_hdr is null...");
    return NULL;
  }

  return ret_shm_junk_elem_hdr;
}


void* wv_shm_pop_elem_data(wv_shm_junk_hdr_t* junk_hdr, wv_shm_junk_elem_hdr_t* elem_hdr)
{
  void* ret_addr_read = 0;
  wv_shm_junk_elem_hdr_t* cur_elem_hdr = NULL;
  size_t offs_read = 0;

  wv_write_log(LOG_INF,"[ %s ]" , __func__);

  if ( junk_hdr &&
       junk_hdr->quu_start_offset &&
       junk_hdr->quu_end_offset &&
       junk_hdr->read_offset &&
       elem_hdr ){

    if(junk_hdr->read_offset == junk_hdr->write_offset){

      wv_write_log(LOG_INF, "The queue was empty...\n");
      return NULL;
    }

    /* Read the header information */
    wv_write_log(LOG_INF, "cur_elem_hdr address : %p", shm_meta->shm_start_addr + junk_hdr->read_offset);

    cur_elem_hdr = shm_meta->shm_start_addr + junk_hdr->read_offset;
    wv_write_log(LOG_INF, "next_offset : %ld", cur_elem_hdr->next_offset);
    wv_write_log(LOG_INF, "size : %ld", cur_elem_hdr->size);

    if (cur_elem_hdr->next_offset && elem_hdr->next_offset ){

      wv_write_log(LOG_INF, "ok?");
      if ( cur_elem_hdr->next_offset != elem_hdr->next_offset ){

	wv_write_log(LOG_ERR,
		     "The pos of a argument hdr was different with the read pos of current header..."
		     "(next_offset_stored : %ld <--> next_offset_wanted : %ld).",
		     cur_elem_hdr->next_offset, elem_hdr->next_offset);

	return NULL;
      }
    }

    wv_write_log(LOG_INF, "ok?");
    if ( cur_elem_hdr->size != elem_hdr->size ){

	wv_write_log(LOG_ERR,
		     "The size of a argument hdr was different with the read size of current header..."
		     "(size_stored : %ld <--> size_wanted : %ld).",
		     cur_elem_hdr->size, elem_hdr->size);

      return NULL;
    }

    offs_read = junk_hdr->read_offset + sizeof(wv_shm_junk_elem_hdr_t);
    ret_addr_read = shm_meta->shm_start_addr + offs_read;

    wv_write_log(LOG_INF, "offset to read : %ld", offs_read);
    wv_write_log(LOG_INF, "address to read : %p", ret_addr_read);

    /* If cur_shm_junk_elem_hdr->next_offset is NULL, this case is the state of the queue. */
    if ( cur_elem_hdr->next_offset){

      /* Set the read postion to the next element position. */
      junk_hdr->read_offset = elem_hdr->next_offset;
      junk_hdr->count--;
      junk_hdr->remain_size -= ( sizeof(wv_shm_junk_elem_hdr_t) + elem_hdr->size );
    }
    else{

      /* This case is the empty state, so make the read position same with the write position. */
      junk_hdr->read_offset = junk_hdr->write_offset;
    }
  }

  return ret_addr_read;
}


/* /\* Read a element from shared memory and memeset() with 0x00 *\/ */
/* int wv_shm_del_last_elem(wv_shm_junk_hdr_t* shm_junk_hdr){ */
/*   int ret = 0; */
/*   wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr = NULL; */

/*   if(shm_junk_hdr && */
/*      shm_junk_hdr->quu_start_addr && */
/*      shm_junk_hdr->quu_end_addr && */
/*      shm_junk_hdr->read_addr) */
/*   { */
/*     shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)wv_shm_peek_elem_hdr(shm_junk_hdr); */

/*     if (shm_junk_elem_hdr->next_addr){ */
/*       shm_junk_hdr->read_addr = shm_junk_elem_hdr->next_addr; */
/*     } */
/*     else{ */
/*       ret = -1; */
/*       fprintf(stdout, "When delete a last element, the next_addr of the element was NULL.\n"); */
/*       goto wv_shm_rm_elem_ret; */
/*     } */
/*   } */
/*   else{ */
/*     ret = -1; */
/*     fprintf(stderr, "When delete a last element, shm_junk_hdr is NULL\n"); */
/*     goto wv_shm_rm_elem_ret; */
/*   } */

/*  wv_shm_rm_elem_ret: */

/*   return ret; */
/* } */


void wv_shm_junk_show(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);
  printf("shm_junk_hdr->count : %ld\n", shm_junk_hdr->count);
  printf("shm_junk_hdr->start_offset : %ld\n", shm_junk_hdr->quu_start_offset);
  printf("shm_junk_hdr->end_offset : %ld\n", shm_junk_hdr->quu_end_offset);
  printf("shm_junk_hdr->remain_size : %ld\n", shm_junk_hdr->remain_size);
  printf("shm_junk_hdr->shm_name : %s\n", shm_junk_hdr->shm_name);
  printf("shm_junk_hdr->is_assigned : %d\n", shm_junk_hdr->is_assigned);

  wv_write_log(LOG_INF, "shm_junk_hdr->shm_name : %s", shm_junk_hdr->shm_name);
  wv_write_log(LOG_INF, "shm_junk_hdr->is_assigned : %d", shm_junk_hdr->is_assigned);
  wv_write_log(LOG_INF, "shm_junk_hdr->start_offset : %ld", shm_junk_hdr->start_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->start_offset : %ld", shm_junk_hdr->start_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->quu_start_offset : %ld", shm_junk_hdr->quu_start_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->quu_end_offset : %ld", shm_junk_hdr->quu_end_offset );
  wv_write_log(LOG_INF, "shm_junk_hdr->write_offset : %ld", shm_junk_hdr->write_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->prev_write_offset : %ld", shm_junk_hdr->prev_write_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr->remain_size : %ld", shm_junk_hdr->remain_size);
  wv_write_log(LOG_INF, "shm_junk_hdr->count : %ld", shm_junk_hdr->count);
}
