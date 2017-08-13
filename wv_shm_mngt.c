#include "wv_shm_mngt.h"
#include <wv_mem.h>


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
  if ( (shm_id = shmget(shm_meta->shm_key, 0, 0)) != -1 )
  {
    shm_exists = 1;
    fprintf(stderr,
	    "[Success] Shared memory was existing.... (shm_id : %d, errno : %s)\n",
	    shm_id, strerror(errno));
    shmctl(shm_id, IPC_RMID, 0);
  }
  else{

    printf("shm_meta->shm_key : %ld\n", shm_meta->shm_key);
    printf("shm_meta->shm_total_size : %ld\n", shm_meta->shm_total_size);

    if ( (shm_id = shmget(shm_meta->shm_key,
			  shm_meta->shm_total_size,
			  IPC_CREAT|0777)) == -1 ){

      perror( "shmget" );
      exit( errno );
      fprintf(stderr,
	      "Getting Shared memory was failed..... (shm_key : %ld, errno : %s)\n",
	      shm_meta->shm_key, strerror(errno));
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

  /* printf( "[Info] loading shm_addr : %p\n", shm_ds.shm_perm */
  printf( "[Info] shm_ds.shm_segsz : %ld\n", shm_ds.shm_segsz);
  printf( "[Info] shm_ds.shm_ctime : %ld\n", shm_ds.shm_ctime);

  /* char* test_message = "eteest"; */

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
  printf("* shm_meta.shm_start_addr : %p\n", shm_meta->shm_start_addr);
  printf("* shm_meta.shm_end_addr : %p\n", shm_meta->shm_end_addr);
  printf("* shm_meta.key : %ld\n", shm_meta->shm_key);
  printf("* shm_meta.shm_junk_size : %ld\n", shm_meta->shm_junk_size);
  printf("* shm_meta.shm_total_size : %ld\n", shm_meta->shm_total_size);

  return ret;
}

int wv_shm_init_meta(void* shm_start_addr)
{

  fprintf(stdout, "[ %s ]\n" , __func__);

  int ret = 0;
  int i = 0;
  size_t page_size, shm_alloc_size = 0;
  void* cur_addr = NULL;

  shm_meta->shm_start_addr = shm_start_addr;
  page_size = getpagesize();
  shm_alloc_size = (SHM_TOTAL_SIZE / page_size) * page_size + page_size;
  shm_meta->shm_end_addr = shm_meta->shm_start_addr + shm_alloc_size;
  shm_meta->shm_total_size = shm_alloc_size;
  memset(shm_meta->shm_start_addr, 0x00, sizeof(shm_alloc_size));

  printf("< The Initialized shared meta data >\n");
  printf("* shm_meta.count : %ld\n", shm_meta->count);
  printf("* shm_meta.shm_start_addr : %p\n", shm_meta->shm_start_addr);
  printf("* shm_meta.shm_end_addr : %p\n", shm_meta->shm_end_addr);
  printf("* shm_meta.key : %ld\n", shm_meta->shm_key);
  printf("* shm_meta.shm_junk_size : %ld\n", shm_meta->shm_junk_size);
  printf("* shm_meta.shm_total_size : %ld\n", shm_meta->shm_total_size);

  /* Split the section of shared memory */
  cur_addr = shm_meta->shm_start_addr;
  shm_meta->count = 0;
  for ( i=0; i < SHM_MAX_COUNT && (cur_addr < shm_meta->shm_end_addr); i++ ){

    wv_shm_junk_init(&shm_meta->arr_shm_junk_hdr[i], "", cur_addr, cur_addr+SHM_JUNK_SIZE);
    cur_addr += SHM_JUNK_SIZE;
    shm_meta->count++;
  }

  /* wv_shm_sync_meta(shm_meta); */

  return ret;
}


int wv_shm_sync_meta(wv_shm_meta_t* shm_meta)
{
  fprintf(stdout, "[ %s ]\n" , __func__);

  int ret = 0;

  fprintf(stdout, "< The synced shared meta data >\n");
  printf("* shm_meta.count : %ld\n", shm_meta->count);
  printf("* shm_meta.shm_start_addr : %p\n", shm_meta->shm_start_addr);
  printf("* shm_meta.shm_end_addr : %p\n", shm_meta->shm_end_addr);
  printf("* shm_meta.key : %ld\n", shm_meta->shm_key);
  printf("* shm_meta.shm_junk_size : %ld\n", shm_meta->shm_junk_size);
  printf("* shm_meta.shm_total_size : %ld\n", shm_meta->shm_total_size);

  if ( shm_meta && wv_shm_wr( shm_meta->shm_start_addr,
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


wv_shm_junk_hdr_t* wv_shm_assign_junk(const char* junk_name)
{
  fprintf(stdout, "[ %s ]\n" , __func__);

  wv_shm_junk_hdr_t* ret_shm_junk_hdr = NULL;
  int i = 0;

  if ( shm_meta && junk_name ){

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
    fprintf(stderr, "The Ware Valley Shared Memory was not initailized...\n" );
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

    ret_junk_hdr_list = (wv_shm_junk_hdr_t**)malloc(sizeof(wv_shm_junk_hdr_t*) * (shm_meta->count + 1));

    for(i = 0; i<shm_meta->count+1; i++){

      (*(ret_junk_hdr_list+i)) = (wv_shm_junk_hdr_t*)malloc(sizeof(wv_shm_junk_hdr_t));
      ((*ret_junk_hdr_list)+i)->count = shm_meta->arr_shm_junk_hdr[i].count;
      ((*ret_junk_hdr_list)+i)->is_assigned = shm_meta->arr_shm_junk_hdr[i].is_assigned;
      ((*ret_junk_hdr_list)+i)->prev_write_addr = shm_meta->arr_shm_junk_hdr[i].prev_write_addr;
      ((*ret_junk_hdr_list)+i)->quu_start_addr = shm_meta->arr_shm_junk_hdr[i].quu_start_addr;
      ((*ret_junk_hdr_list)+i)->quu_end_addr = shm_meta->arr_shm_junk_hdr[i].quu_end_addr;
      ((*ret_junk_hdr_list)+i)->read_addr = shm_meta->arr_shm_junk_hdr[i].read_addr;
      ((*ret_junk_hdr_list)+i)->remain_size = shm_meta->arr_shm_junk_hdr[i].remain_size;
      snprintf(((*ret_junk_hdr_list)+i)->shm_name,
	       256, "%s", shm_meta->arr_shm_junk_hdr[i].shm_name);
      ((*ret_junk_hdr_list)+i)->start_addr = shm_meta->arr_shm_junk_hdr[i].start_addr;
      ((*ret_junk_hdr_list)+i)->write_addr = shm_meta->arr_shm_junk_hdr[i].write_addr;
    } 
  }
  else{
    fprintf(stderr, "shm_meta was not initialized...");
  }

  return ret_junk_hdr_list;
}


wv_shm_junk_hdr_t**  wv_shm_free_junk_list(const wv_shm_junk_hdr_t* const* junk_hdr_list){

  int i = 0;

}


int wv_shm_clear_junk(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  fprintf(stdout, "[ %s ]\n" , __func__);

  int ret = 0;
  if ( shm_junk_hdr &&
       shm_junk_hdr->quu_start_addr &&
       shm_junk_hdr->quu_end_addr )
  {
    memset(shm_junk_hdr->quu_start_addr, 0x00 , shm_junk_hdr->quu_end_addr - shm_junk_hdr->quu_start_addr);

    shm_junk_hdr->write_addr = shm_junk_hdr->quu_start_addr;
    shm_junk_hdr->read_addr = shm_junk_hdr->quu_start_addr;
    shm_junk_hdr->prev_write_addr = NULL;
    shm_junk_hdr->remain_size =  shm_junk_hdr->quu_end_addr - shm_junk_hdr->quu_start_addr;
    shm_junk_hdr->count = 0;
  }
  else{
    ret = 1;
    fprintf(stderr, "When cleared the shared memory junk, we received a null value.\n");
  }

  return ret;
}


int wv_shm_dump_junk(const char* junk_name, const char* dir_name, const char* file_name)
{
  fprintf(stdout, "[ %s ]\n" , __func__);

  int ret = 0;
  int wr_buf_size = 8192;
  char full_path[8192] = {0};
  void* cur_pos = NULL;
  FILE* pfile = NULL;
  const char* dlmt = "/";
  wv_shm_junk_hdr_t* shm_junk_hdr = NULL;
  int nwrite = 0;

  snprintf( full_path, 8192, "%s%s%s", dir_name, dlmt, file_name );

  /* Find if there is shm_junk existing or not. */
  if ( (shm_junk_hdr = wv_shm_find_junk(shm_meta, junk_name)) == NULL ){

    fprintf( stdout, "Counldnt find the shared memory junk having same name...\n" );
    ret = -1; goto wv_shm_dump_junk_ret;
  }

  /* Find if there is a file having the same name or not. */
  if ( access(full_path, F_OK) == 0 ){

    fprintf( stdout, "There is a file existing...\n" );
    ret = -1; goto wv_shm_dump_junk_ret;
  }
  else{

    /* Dump the founded shm_junk to a file. */
    if ( (pfile = fopen(full_path, "wb")) == NULL ){

      fprintf( stdout, "Couldnt open the file with wb mode...\n" );
      ret = -1; goto wv_shm_dump_junk_ret;
    }
  }

  /* Dump the founed shm_junk to a file. */
  if ( shm_junk_hdr->start_addr ){
    cur_pos = shm_junk_hdr->start_addr;

    while( 1 ){

      if ((cur_pos + wr_buf_size) > shm_junk_hdr->quu_end_addr){

	fwrite(cur_pos, sizeof(char), shm_junk_hdr->quu_end_addr - cur_pos, pfile);
      }
      else{

	fwrite(cur_pos, sizeof(char), wr_buf_size, pfile);
	break;
      }
      cur_pos += wr_buf_size;
    }
  }

  if (pfile){

    fclose(pfile);
  }


 wv_shm_dump_junk_ret:

  return ret;
}



int wv_shm_load_junk(wv_shm_meta_t* shm_meta, const char* dir_name, const char* file_name)
{
  int ret = 0;
  return ret;
}


int wv_shm_junk_init(wv_shm_junk_hdr_t* shm_junk_hdr, char* shm_junk_name, void* start_addr, void* end_addr)
{
  fprintf(stdout, "[ %s ]\n" , __func__);

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

  /* Write the junk header information to shared memory */
  shm_junk_hdr->start_addr = start_addr;

  wv_shm_wr(start_addr, shm_junk_hdr, sizeof(wv_shm_junk_hdr_t), &junk_start_addr, end_addr);

  shm_junk_hdr->quu_start_addr = junk_start_addr;
  shm_junk_hdr->quu_end_addr = end_addr;
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

  /* fprintf(stdout, "< Inner wv_shm_wr  >\n"); */
  /* printf("* shm_meta start addr : %p\n", start_addr); */
  /* printf("* shm_meta data : %p\n", data); */
  /* printf("* shm_meta size : %ld\n", size); */

  if (start_addr && limit_addr && data)
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

    printf("===========================\n");
    printf("* start addr to write : %p\n", start_addr);
    printf("* shm_meta data : %p\n", data);
    printf("* shm_meta size : %ld\n", size);

    if ((alloc_addr = memcpy(start_addr, data, size)))
    {
      if ( next_addr ) { *next_addr = alloc_addr +size; }
    }
    else
    {
      fprintf(stderr, "memset() was failed.... (errono : %s)\n", strerror(errno));
    }
  }

 wv_shm_wr_elem_ret:

  return alloc_addr;
}


void* wv_shm_push_elem(wv_shm_junk_hdr_t* shm_junk_hdr, void* data, size_t size)
{
  void* alloc_addr = NULL;
  wv_shm_junk_elem_hdr_t shm_junk_elem_hdr;

  fprintf(stdout, "[ %s ]\n" , __func__);

  if (shm_junk_hdr &&
      shm_junk_hdr->write_addr &&
      shm_junk_hdr->quu_start_addr &&
      shm_junk_hdr->quu_end_addr)
  {
    if ( shm_junk_hdr->write_addr < shm_junk_hdr->quu_start_addr )
    {
      fprintf(stdout, "write area was lower than end address..\n");
      goto wv_shm_wr_elem_ret;
    }

    if ( (shm_junk_hdr->write_addr + size) > shm_junk_hdr->quu_end_addr )
    {
      fprintf(stdout, "write area was higher than end address..\n");
      goto wv_shm_wr_elem_ret;
    }

    /* Check the write address exceed junk_memory or not and make it coreccted. */

    /* +-------------------------+        +-------------------------+ */
    /* |                   elem_data  =>  |elem_data                | */
    /* +-------------------------+        +-------------------------+ */

    if ( ( shm_junk_hdr->write_addr + sizeof(wv_shm_junk_elem_hdr_t) + size ) > shm_junk_hdr->quu_end_addr ){
      shm_junk_hdr->write_addr = shm_junk_hdr->quu_start_addr;
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

      fprintf(stdout, "->pos_write_hdr : %p\n", alloc_addr);

      shm_junk_hdr->write_addr += sizeof(wv_shm_junk_elem_hdr_t);
    }
    else
    {
	fprintf(stdout, "[Error] memcpy() was failed when write a elem_header.");
	goto wv_shm_wr_elem_ret;
    }

    /* Write the element data information */
    /* if ((alloc_addr = memcpy(shm_junk_hdr->write_addr, data, size))) */

    printf(">>>shared emem header size : %ld\n", sizeof(wv_shm_junk_elem_hdr_t));
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

      fprintf(stdout, "->pos_write_data : %p\n", alloc_addr);

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

  fprintf(stdout, "shm_junk_hdr->read_addr : %p\n", shm_junk_hdr->read_addr);

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

    if ( next_addr ) { *next_addr = start_addr + size; }
    
    ret = start_addr;
  }

  return ret;
}


wv_shm_junk_elem_hdr_t* wv_shm_peek_elem_hdr(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr = NULL;

  fprintf(stdout, "[ %s ]\n" , __func__);

  if (shm_junk_hdr &&
      shm_junk_hdr->quu_start_addr &&
      shm_junk_hdr->quu_end_addr &&
      shm_junk_hdr->read_addr){

    shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)shm_junk_hdr->read_addr;
  }
  else{
    fprintf(stderr, "!! a shm_hdr or its attrs is null...");
  }

  return shm_junk_elem_hdr;
}


/* Return the read addres and add a size value to the read address */
void* wv_shm_peek_elem_data(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  wv_shm_junk_elem_hdr_t* ret_shm_junk_elem_hdr = NULL;
  void* pos_read = NULL;

  fprintf(stdout, "[ %s ]\n" , __func__);

  if ( shm_junk_hdr &&
       shm_junk_hdr->quu_start_addr &&
       shm_junk_hdr->quu_end_addr &&
       shm_junk_hdr->read_addr )
  {

    pos_read = shm_junk_hdr->read_addr + sizeof(wv_shm_junk_elem_hdr_t);

    fprintf(stdout, "-> shm_junk_hdr->read_addr : %p..\n", shm_junk_hdr->read_addr);
    fprintf(stdout, "-> pos_read : %p..\n", pos_read);

    ret_shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)pos_read;
  }
  else{

    fprintf(stderr, "!! a shm_hdr or shm_junk_elem_hdr is null...");
    return NULL;
  }

  return ret_shm_junk_elem_hdr;
}


void* wv_shm_pop_elem_data(wv_shm_junk_hdr_t* shm_junk_hdr, wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr)
{
  void* ret = NULL;
  wv_shm_junk_elem_hdr_t* cur_shm_junk_elem_hdr = NULL;

  fprintf(stdout, "[ %s ]\n" , __func__);

  if ( shm_junk_hdr &&
       shm_junk_hdr->quu_start_addr &&
       shm_junk_hdr->quu_end_addr &&
       shm_junk_hdr->read_addr &&
       shm_junk_elem_hdr ){

    if(shm_junk_hdr->read_addr == shm_junk_hdr->write_addr){

      fprintf(stderr, "** The queue was empty...\n");
      return NULL;
    }

    ret = shm_junk_hdr->read_addr + sizeof(wv_shm_junk_elem_hdr_t);

    cur_shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*) shm_junk_hdr->read_addr;

    if ( shm_junk_elem_hdr->next_addr ){

      if ( cur_shm_junk_elem_hdr->next_addr != shm_junk_elem_hdr->next_addr ){

	fprintf(stderr,
		"!! The pos of a argument hdr was different with the read pos of current header..."
		"(size_stored : %p <--> size_wanted : %p)\n",
		cur_shm_junk_elem_hdr->next_addr, shm_junk_elem_hdr->next_addr);

	return NULL;
      }
    }

    if ( cur_shm_junk_elem_hdr->size != shm_junk_elem_hdr->size ){

      fprintf(stderr,
	      "!! The size of a argument hdr was different with the read size of current header..."
	      "(size_stored : %ld <--> size_wanted : %ld)\n",
	      cur_shm_junk_elem_hdr->size, shm_junk_elem_hdr->size);

      return NULL;
    }

    fprintf(stdout, ">>> %p\n", shm_junk_hdr->write_addr);
    if ( shm_junk_elem_hdr->next_addr){

      shm_junk_hdr->read_addr = shm_junk_elem_hdr->next_addr;
      shm_junk_hdr->count--;
      shm_junk_hdr->remain_size -= ( sizeof(wv_shm_junk_elem_hdr_t) + shm_junk_elem_hdr->size );
    }
    else{

      shm_junk_hdr->read_addr = shm_junk_hdr->write_addr;
    }
  }

  return ret;
}


/* Read a element from shared memory and memeset() with 0x00 */
int wv_shm_del_last_elem(wv_shm_junk_hdr_t* shm_junk_hdr){
  int ret = 0;
  wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr = NULL;

  if(shm_junk_hdr &&
     shm_junk_hdr->quu_start_addr &&
     shm_junk_hdr->quu_end_addr &&
     shm_junk_hdr->read_addr)
  {
    shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)wv_shm_peek_elem_hdr(shm_junk_hdr);

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


void wv_shm_junk_show(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  fprintf(stdout, "[ %s ]\n" , __func__);

  printf("shm_junk_hdr->count : %ld\n", shm_junk_hdr->count);
  printf("shm_junk_hdr->start_addr : %p\n", shm_junk_hdr->quu_start_addr);
  printf("shm_junk_hdr->end_addr : %p\n", shm_junk_hdr->quu_end_addr);
  printf("shm_junk_hdr->remain_size : %ld\n", shm_junk_hdr->remain_size);
  printf("shm_junk_hdr->shm_name : %s\n", shm_junk_hdr->shm_name);
  printf("shm_junk_hdr->is_assigned : %d\n", shm_junk_hdr->is_assigned);
}
