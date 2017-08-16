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
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

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
  if ( (shm_id = shmget(shm_meta->shm_key,
			shm_meta->shm_total_size,
			IPC_CREAT | IPC_EXCL)) == -1 ){

    wv_write_log(LOG_INF,
		 "Previous shared memory was existing... (shm_id : %d, errno : %s)",
		 shm_id, strerror(errno));

    if ( (shm_id = shmget(shm_meta->shm_key,
			  shm_meta->shm_total_size,
			  0)) == -1 ){
      wv_write_log(LOG_INF,
		   "Getting the previous shared memory faild... (shm_id : %d, errno : %s)",
		   shm_id, strerror(errno));
    }
    else{
      shm_exists = 1;
      /* shmctl(shm_id, IPC_RMID, 0); */
    }
    /* exit( errno ); */
  }
  else{
    wv_write_log(LOG_INF, "shm_meta->shm_key : %ld", shm_meta->shm_key);
    wv_write_log(LOG_INF, "shm_meta->shm_total_size : %ld", shm_meta->shm_total_size);

    if ( (shm_id = shmget(shm_meta->shm_key,
			  shm_meta->shm_total_size,
			  IPC_CREAT|0777)) == -1 ){

      wv_write_log(LOG_ERR, "Getting Shared memory was failed..... (shm_key : %ld, errno : %s)",
	      shm_meta->shm_key, strerror(errno));

      exit( errno );
      return -1;
    }
  }

  /* Attaching shared memory. */
  shm_addr = shmat(shm_id, (void*)0, 0);
  if ( shm_addr == (void*)(-1) )
  {
    wv_write_log(LOG_ERR, "Getting sharedaddr : %p", shm_meta->shm_start_addr);
    wv_write_log(LOG_ERR, "cur_addr : %p", cur_addr);
    return -1;
  }

  if (shmctl(shm_id, IPC_STAT, &shm_ds) == -1){

    wv_write_log(LOG_ERR, "[Error] Getting shred info failed...");
    return -1;
  }

  wv_write_log(LOG_INF, "shm_ds.shm_segsz : %ld", shm_ds.shm_segsz);
  wv_write_log(LOG_INF, "shm_ds.shm_ctime : %ld", shm_ds.shm_ctime);

  shm_exists = 1;
  /* shm_exists = 0; */
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


int wv_shm_load_meta(void* shm_start_addr)
{
  int ret = 0;
  wv_write_log(LOG_INF, "[ %s ]", __func__);

  if (!wv_shm_check_init()){
    return ret = -1;
  }

  if (shm_start_addr == NULL){
    return ret = -1;
  }

  shm_meta = (wv_shm_meta_t*)wv_shm_rd(shm_start_addr, sizeof(wv_shm_meta_t), NULL);
  shm_meta->shm_start_addr = shm_start_addr;
  shm_meta->shm_end_addr = shm_start_addr + shm_meta->shm_total_size;

  wv_write_log(LOG_INF, "<The loaded shared meta data >");
  wv_write_log(LOG_INF, "* shm_meta.count : %ld", shm_meta->count);
  wv_write_log(LOG_INF, "* shm_meta.shm_start_addr : %p", shm_start_addr);
  wv_write_log(LOG_INF, "* shm_meta.shm_end_addr : %p", shm_start_addr + shm_meta->shm_total_size);
  wv_write_log(LOG_INF, "* shm_meta.key : %ld", shm_meta->shm_key);
  wv_write_log(LOG_INF, "* shm_meta.shm_junk_size : %ld", shm_meta->shm_junk_size);
  wv_write_log(LOG_INF, "* shm_meta.shm_total_size : %ld", shm_meta->shm_total_size);

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
    wv_shm_junk_init(NULL, cur_offs, cur_offs + SHM_JUNK_SIZE);
    shm_meta->arr_junk_hdr_offsets[i] = cur_offs;
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

  wv_write_log(LOG_INF, "<The initailized shared meta data >");
  wv_write_log(LOG_INF, "* shm_meta.count : %ld", shm_meta->count);
  wv_write_log(LOG_INF, "* shm_meta.shm_start_addr : %p", shm_meta->shm_start_addr);
  wv_write_log(LOG_INF, "* shm_meta.shm_end_addr : %p", shm_meta->shm_start_addr + shm_meta->shm_total_size);
  wv_write_log(LOG_INF, "* shm_meta.key : %ld", shm_meta->shm_key);
  wv_write_log(LOG_INF, "* shm_meta.shm_junk_size : %ld", shm_meta->shm_junk_size);
  wv_write_log(LOG_INF, "* shm_meta.shm_total_size : %ld", shm_meta->shm_total_size);

  if ( shm_meta && wv_shm_wr( 0,
			      shm_meta,
			      sizeof(wv_shm_meta_t),
			      NULL ) == NULL ){

    wv_write_log(LOG_ERR, "When wrote the shared memory meta data to the shared memory, error occured. ");
    return ret = 1;
  }

  return ret;
}


wv_shm_junk_hdr_t* wv_shm_find_junk(const char* junk_name)
{
  wv_shm_junk_hdr_t* ret_pshm_junk_hdr = NULL;
  int i = 0;

  if ( shm_meta ){
    for(i = 0; i < shm_meta->count; i++){

      ret_pshm_junk_hdr = shm_meta->shm_start_addr + shm_meta->arr_junk_hdr_offsets[i];

      if( ret_pshm_junk_hdr->is_assigned &&
	  strncmp(ret_pshm_junk_hdr->shm_name, junk_name, strlen(junk_name)) == 0 )
	{
	  return ret_pshm_junk_hdr;
	}
    }
  }

  return ret_pshm_junk_hdr = NULL;
}


wv_shm_junk_hdr_t* wv_shm_assign_junk(const char* junk_name)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  wv_shm_junk_hdr_t* ret_pshm_junk_hdr = NULL;
  int i = 0;

  if ( shm_meta && junk_name ){

    wv_write_log(LOG_INF, "The count of shm_meta : %d" , shm_meta->count);

    for ( i = 0; i < shm_meta->count; i++ ){
      wv_write_log(LOG_INF,"shm_meta->arr_junk_hdr_offsets[0] : %ld", shm_meta->arr_junk_hdr_offsets[i]);
      wv_write_log(LOG_INF,"shm_meta->arr_junk_hdr_offsets[1] : %ld", shm_meta->arr_junk_hdr_offsets[i]);

      ret_pshm_junk_hdr = shm_meta->shm_start_addr + shm_meta->arr_junk_hdr_offsets[i];
      wv_write_log(LOG_INF,"shm_junk_hdr : %p", ret_pshm_junk_hdr);
      wv_write_log(LOG_INF,"shm_junk_hdr->quu_start_offset : %ld", ret_pshm_junk_hdr->quu_start_offset);
      wv_write_log(LOG_INF,"shm_junk_hdr->quu_end_offset : %ld", ret_pshm_junk_hdr->quu_end_offset);

      if(wv_shm_find_junk(junk_name)){

	wv_write_log(LOG_WRN, "When the shared memory junk assigned, "
		     "The junk having the same name was existing.");
	ret_pshm_junk_hdr = NULL; goto ret_wv_shm_assign_junk;
      }

      if ( ret_pshm_junk_hdr->is_assigned == 0 ){

      	ret_pshm_junk_hdr->is_assigned = 1;
      	snprintf(ret_pshm_junk_hdr->shm_name, 256, "%s", junk_name);

	wv_write_log(LOG_INF, "%p", ret_pshm_junk_hdr);
      	wv_shm_clear_junk(ret_pshm_junk_hdr);

      	return ret_pshm_junk_hdr;
      }
      else{

	wv_write_log(LOG_INF, "When we try to assign the shared memory junk,"
		     "the is_assigned attribute was 1 (it means assigned...)");
	ret_pshm_junk_hdr = NULL; goto ret_wv_shm_assign_junk;
      }
    }
  }
  else{

    wv_write_log(LOG_ERR, "The Ware Valley Shared Memory was not initailized..." );
  }

 ret_wv_shm_assign_junk:

  return ret_pshm_junk_hdr;
}


wv_shm_junk_hdr_t* wv_shm_unassign_junk(wv_shm_meta_t* shm_meta, const char* junk_name)
{
  wv_write_log(LOG_INF, "[ %s ]\n" , __func__);

  if (shm_meta == NULL){
    wv_write_log(LOG_INF, "Please initailize the shared memory with wv_shm_init()...");
    return NULL;
  }

  wv_shm_junk_hdr_t* ret_pshm_junk_hdr = NULL;
  int i = 0;

  if ( shm_meta ){

    for ( i = 0; i < shm_meta->count; i++ ){
      ret_pshm_junk_hdr = shm_meta->shm_start_addr + shm_meta->arr_junk_hdr_offsets[i];

      if ( ret_pshm_junk_hdr->is_assigned == 1  &&
	   strncmp(ret_pshm_junk_hdr->shm_name, junk_name, strlen(junk_name)) == 0 ){

	ret_pshm_junk_hdr->is_assigned = 0;
	strcpy(ret_pshm_junk_hdr->shm_name, "");

	wv_shm_clear_junk(ret_pshm_junk_hdr);

	return ret_pshm_junk_hdr;
      }
    }
  }

  return ( ret_pshm_junk_hdr = NULL );
}



wv_shm_junk_hdr_t** wv_shm_get_junk_list(){

  wv_shm_junk_hdr_t** ret_junk_hdr_list = NULL;
  wv_shm_junk_hdr_t* pjunk_hdr = NULL;
  int i = 0;

  if (!wv_shm_check_init()){
    return NULL;
  }

  ret_junk_hdr_list = (wv_shm_junk_hdr_t**)malloc(sizeof(wv_shm_junk_hdr_t*) * (shm_meta->count + 1));
  memset(ret_junk_hdr_list, 0x00, sizeof(wv_shm_junk_hdr_t*) * (shm_meta->count + 1));

  for(i = 0; i<shm_meta->count; i++){
    pjunk_hdr = shm_meta->shm_start_addr + shm_meta->arr_junk_hdr_offsets[i];

    (*(ret_junk_hdr_list+i)) = (wv_shm_junk_hdr_t*)malloc(sizeof(wv_shm_junk_hdr_t));
    snprintf((*(ret_junk_hdr_list+i))->shm_name, 256, "%s", pjunk_hdr->shm_name);
    (*(ret_junk_hdr_list+i))->count = pjunk_hdr->count;
    (*(ret_junk_hdr_list+i))->is_assigned = pjunk_hdr->is_assigned;
    (*(ret_junk_hdr_list+i))->prev_write_offset = pjunk_hdr->prev_write_offset;
    (*(ret_junk_hdr_list+i))->quu_start_offset = pjunk_hdr->quu_start_offset;
    (*(ret_junk_hdr_list+i))->quu_end_offset = pjunk_hdr->quu_end_offset;
    (*(ret_junk_hdr_list+i))->read_offset = pjunk_hdr->read_offset;
    (*(ret_junk_hdr_list+i))->remain_size = pjunk_hdr->remain_size;
    (*(ret_junk_hdr_list+i))->start_offset = pjunk_hdr->start_offset;
    (*(ret_junk_hdr_list+i))->write_offset = pjunk_hdr->write_offset;
  }

  (*(ret_junk_hdr_list + (shm_meta->count+1))) = NULL;

  return ret_junk_hdr_list;
}


void wv_shm_free_junk_list(wv_shm_junk_hdr_t** junk_hdr_list){

  int i = 0;

  if ( junk_hdr_list ){

    for (i = 0; junk_hdr_list[i]; i++){

      if(junk_hdr_list[i]){

	free(junk_hdr_list[i]);
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

  if ( !wv_shm_check_init() ){

    ret = -1;

    return ret;
  }

  if ( shm_junk_hdr ){

    if ( shm_junk_hdr &&
	 shm_junk_hdr->quu_start_offset &&
	 shm_junk_hdr->quu_end_offset ){

	memset( (shm_meta->shm_start_addr + shm_junk_hdr->quu_start_offset),
	        0x00,
		shm_junk_hdr->quu_end_offset - shm_junk_hdr->quu_start_offset );

	shm_junk_hdr->write_offset = shm_junk_hdr->quu_start_offset;
	shm_junk_hdr->read_offset = shm_junk_hdr->quu_start_offset;
	shm_junk_hdr->prev_write_offset = 0;
	shm_junk_hdr->remain_size =  shm_junk_hdr->quu_end_offset - shm_junk_hdr->quu_start_offset;
	shm_junk_hdr->count = 0;

	wv_write_log(LOG_INF,"shm_junk_hdr : %p", shm_junk_hdr);
	wv_write_log(LOG_INF,"shm_junk_hdr->quu_start_offset : %ld", shm_junk_hdr->quu_start_offset);
	wv_write_log(LOG_INF,"shm_junk_hdr->quu_end_offset : %ld", shm_junk_hdr->quu_end_offset);
    }
    else{

      wv_write_log(LOG_INF, "When cleared the shared memory junk, shm_junk_hdr was NULL...\n");
      ret = -1;

      return ret;
    }
  }


  return ret;
}


int wv_shm_dump_junk(const char* junk_name, const char* dir_name, const char* file_name)
{
  wv_write_log(LOG_INF, "[ %s ]\n" , __func__);

  int ret = 0;
  int wr_buf_size = 8192;
  char full_path[8192] = {0};
  void* cur_pos = NULL;
  FILE* pfile = NULL;
  const char* dlmt = "/";
  wv_shm_junk_hdr_t* junk_hdr = NULL;
  int nwrite = 0;


  /* Find if there is shm_junk existing or not. */
  if ( (junk_hdr = wv_shm_find_junk(junk_name)) == NULL ){

    wv_write_log(LOG_INF, "Counldnt find the shared memory junk having same name..." );
    ret = -1; goto wv_shm_dump_junk_ret;
  }

  /* Find if there is a file having the same name or not. */
  snprintf( full_path, 8192, "%s%s%s", dir_name, dlmt, file_name );
  wv_write_log(LOG_INF, "Checking if there is the file existing...(file path : %s)", full_path);
  if ( access(full_path, F_OK) == 0 ){

    wv_write_log(LOG_INF, "There is a file existing..." );
    ret = -1; goto wv_shm_dump_junk_ret;
  }
  else{

    /* Dump the founded shm_junk to a file. */
    if ( (pfile = fopen(full_path, "wb")) == NULL ){

      wv_write_log(LOG_INF, "Couldnt open the file with wb mode..." );
      ret = -1; goto wv_shm_dump_junk_ret;
    }
  }

  /* Dump the founed shm_junk to a file. */
  if ( junk_hdr->start_offset ){

    cur_pos = shm_meta->shm_start_addr+ junk_hdr->start_offset;

    while( 1 ){

      if ((cur_pos + wr_buf_size) > shm_meta->shm_start_addr + junk_hdr->quu_end_offset){

	fwrite(cur_pos, sizeof(char), (shm_meta->shm_start_addr + junk_hdr->quu_end_offset) - cur_pos, pfile);
	break;
      }
      else{

	nwrite = fwrite(cur_pos, sizeof(char), wr_buf_size, pfile);
      }
      cur_pos += nwrite;
    }
  }

  if (pfile){

    fclose(pfile);
  }


 wv_shm_dump_junk_ret:

  return ret;
}


int wv_shm_load_junk(const char* dir_name, const char* file_name)
{
  int ret = 0;
  int i = 0;
  char file_path[8192] = {0};

  char* delimit = "/";
  FILE* pfile_dump = NULL;
  size_t size_file = 0;
  int idx_assigned_junk = 0;

  void *addr_start = NULL;
  void *addr_end = NULL;
  void *addr_curr = NULL;

  int nread = 0;

  wv_shm_junk_hdr_t* pjunk_hdr = NULL;

  if (shm_meta){

    /* Check if there is file existing. */
    snprintf(file_path, 8192, "%s%s%s", dir_name, delimit, file_name);

    if (access(dir_name, F_OK) != 0){
    
      wv_write_log(LOG_ERR, "There is a file not exists... (file_path : %s)", file_path);
      ret = -1; goto wv_shm_load_junk;
    }

    /* Open the dump file. */
    if ((pfile_dump = fopen(file_path, "rb")) == NULL){

      wv_write_log(LOG_ERR, "Opening the file was failed... (file_path : %s)", file_path);
      ret = -1; goto wv_shm_load_junk;
    }

    /* Check if the size of the dump file is larger then the reserved junk size. */
    size_file = fseek(pfile_dump, 0, SEEK_END);
    if (shm_meta->shm_junk_size < size_file){

      wv_write_log(LOG_ERR, "Opening the file was failed... (file_path : %s)", file_path);

      if (pfile_dump){

	fclose(pfile_dump);
      }
      ret = -1; goto wv_shm_load_junk;
    
    }
    fseek(pfile_dump, 0, SEEK_SET);

    for ( i=0; i<shm_meta->count; i++ ){

      pjunk_hdr = shm_meta->shm_start_addr + shm_meta->arr_junk_hdr_offsets[i];

      idx_assigned_junk = i;


      if ( pjunk_hdr->is_assigned == 0 ){

	pjunk_hdr->is_assigned = 1;
	wv_shm_clear_junk(pjunk_hdr);

	snprintf(pjunk_hdr->shm_name, 256, "%s", file_name);
	addr_end = shm_meta->shm_start_addr + pjunk_hdr->quu_end_offset;
	addr_start = shm_meta->shm_start_addr + pjunk_hdr->start_offset;
	addr_curr = addr_start;

      }
    }


    while((nread = fread(addr_curr, 8192, sizeof(char), pfile_dump)) > 0){ }
  }

 wv_shm_load_junk:
  return ret;
}


int wv_shm_junk_init(char* shm_junk_name, size_t start_offs, size_t end_offs)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  int ret = 0;

  int available_size = end_offs - start_offs;

  wv_shm_junk_hdr_t junk_hdr;
  wv_shm_junk_hdr_t* pjunk_hdr = NULL;

  memset(&junk_hdr, 0x00, sizeof(wv_shm_junk_hdr_t));

  if ( available_size < sizeof(wv_shm_junk_hdr_t) )
  {
    ret = -1;
    wv_write_log(LOG_ERR, "When write shared memory junk header to shared memory, "
		 "the structure was bigger than the available size...");
    goto wv_shm_junk_init_ret;
  }

  /* Write the junk header information to shared memory */
  junk_hdr.start_offset = start_offs;
  junk_hdr.quu_start_offset = start_offs + sizeof(wv_shm_junk_hdr_t);
  junk_hdr.quu_end_offset = end_offs;
  junk_hdr.write_offset = junk_hdr.read_offset = start_offs;
  junk_hdr.prev_write_offset = 0;
  junk_hdr.remain_size = end_offs - (start_offs + sizeof(wv_shm_junk_hdr_t));
  junk_hdr.count = 0;

  if (wv_shm_wr(start_offs, &junk_hdr, sizeof(wv_shm_junk_hdr_t), NULL) == NULL){

    ret = -1;
    wv_write_log(LOG_ERR, "Writing the shared memory junk was failed...");
    goto wv_shm_junk_init_ret;
  }

  pjunk_hdr = shm_meta->shm_start_addr + start_offs;

  wv_write_log(LOG_INF, "addr of shm_junk : %p", shm_meta->shm_start_addr + pjunk_hdr->start_offset);
  wv_write_log(LOG_INF, "junk_hdr->shm_name : %s", pjunk_hdr->shm_name);
  wv_write_log(LOG_INF, "junk_hdr->is_assigned : %d", pjunk_hdr->is_assigned);
  wv_write_log(LOG_INF, "junk_hdr->start_offset : %ld", pjunk_hdr->start_offset);
  wv_write_log(LOG_INF, "junk_hdr->quu_start_offset : %ld", pjunk_hdr->quu_start_offset);
  wv_write_log(LOG_INF, "junk_hdr->quu_end_offset : %ld", pjunk_hdr->quu_end_offset );
  wv_write_log(LOG_INF, "junk_hdr->write_offset : %ld", pjunk_hdr->write_offset);
  wv_write_log(LOG_INF, "junk_hdr->prev_write_offset : %ld", pjunk_hdr->prev_write_offset);
  wv_write_log(LOG_INF, "junk_hdr->remain_size : %ld", pjunk_hdr->remain_size);
  wv_write_log(LOG_INF, "junk_hdr->count : %ld", pjunk_hdr->count);

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
  void* ret_alloc_addr = NULL;
  wv_shm_junk_elem_hdr_t elem_hdr;
  void* write_addr = NULL;
  size_t offs_after_write = 0;

  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  if (shm_meta == NULL){
    wv_write_log(LOG_INF, "Please initailize the shared memory with wv_shm_init()...");
    ret_alloc_addr = NULL;
    goto wv_shm_wr_elem_ret;
  }

  wv_write_log(LOG_INF, "shm_junk_hdr : %p", shm_junk_hdr);
  wv_write_log(LOG_INF, "shm_junk_hdr : %ld", shm_junk_hdr->start_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr : %ld", shm_junk_hdr->write_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr : %ld", shm_junk_hdr->quu_start_offset);
  wv_write_log(LOG_INF, "shm_junk_hdr : %ld", shm_junk_hdr->quu_end_offset);
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
    if ( shm_junk_hdr->prev_write_offset ){

      wv_shm_link_elems(shm_junk_hdr,
      			shm_junk_hdr->prev_write_offset,
      			shm_junk_hdr->write_offset);

    }

    wv_write_log(LOG_INF, "shm_meta->shm_start_addr : %p", shm_meta->shm_start_addr);
    wv_write_log(LOG_INF, "shm_junk_hdr->write_offset : %ld", shm_junk_hdr->write_offset);

    shm_junk_hdr->prev_write_offset = shm_junk_hdr->write_offset;

    /* Write the element header information */
    memset(&elem_hdr, 0x00, sizeof(wv_shm_junk_elem_hdr_t));
    elem_hdr.size = size;
    elem_hdr.next_offset = 0;
    wv_write_log(LOG_INF, "elem_hdr.size : %ld", elem_hdr.size);
    wv_write_log(LOG_INF, "elem_hdr.next_offset : %ld", elem_hdr.next_offset);

    write_addr = shm_meta->shm_start_addr + shm_junk_hdr->write_offset;
    wv_write_log(LOG_INF, "write position : %p", write_addr);

    wv_write_log(LOG_INF, "ok?...");

    if ((ret_alloc_addr = memcpy(write_addr,
			     &elem_hdr,
			     sizeof(wv_shm_junk_elem_hdr_t))))
    {
      if ( ret_alloc_addr != shm_meta->shm_start_addr + shm_junk_hdr->write_offset ){

	ret_alloc_addr = NULL;
	wv_write_log(LOG_ERR,
		     "The returned value of malloc() and head->write_addr was not equal.."
		     "(alloc_addr : %p <--> write_addr : %p)",
		     ret_alloc_addr, write_addr);

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
    if ((ret_alloc_addr = memcpy(write_addr, data, size)))
    {
      // debug message
      if ( ret_alloc_addr != write_addr )
      {
	ret_alloc_addr = NULL;
	wv_write_log(LOG_ERR,
		"The returned value of malloc() and head->write_addr was not equal..\
                 (alloc_addr : %p <--> write_addr : %p)",
		ret_alloc_addr, write_addr);

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
  else{
    wv_write_log(LOG_ERR, "The argument shm_junk_hdr was NULL.");
  }

 wv_shm_wr_elem_ret:

  return ret_alloc_addr;
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

    if (shm_junk_hdr->count > 0){

      offset_read = shm_junk_hdr->read_offset;
      addr_read = shm_meta->shm_start_addr + offset_read;

      wv_write_log(LOG_INF, "offset to read : %ld", offset_read);
      wv_write_log(LOG_INF, "address to read :  %p", addr_read);

      ret_elem_hdr = (wv_shm_junk_elem_hdr_t*)addr_read;
      wv_write_log(LOG_INF, "ret_elem_hdr->size : %ld", ret_elem_hdr->size);
      wv_write_log(LOG_INF, "ret_elem_hdr->next_offset : %ld", ret_elem_hdr->next_offset);

    }
    else{

      wv_write_log(LOG_INF, "The queue was empty.");
      return NULL;
    }
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

    if (shm_junk_hdr->count > 0){

      offset_read = shm_junk_hdr->read_offset + sizeof(wv_shm_junk_elem_hdr_t);
      addr_read = shm_meta->shm_start_addr + offset_read;

      wv_write_log(LOG_INF, "offset to read : %ld", offset_read);
      wv_write_log(LOG_INF, "address to read : %p", addr_read);

      ret_shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)addr_read;
    }
    else{

      wv_write_log(LOG_INF, "The queue was empty.");
      return NULL;
    }
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

    if(junk_hdr->count == 0){

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
      if (junk_hdr->count > 0){

	junk_hdr->count--;
	junk_hdr->read_offset = elem_hdr->next_offset;
	junk_hdr->remain_size -= ( sizeof(wv_shm_junk_elem_hdr_t) + elem_hdr->size );
      }
      else{
	wv_write_log(LOG_ERR, "This is weird case, although the next_offset existed"
		     "but header->count was bigger than 0.");
	ret_addr_read = NULL;
      }
    }
    else{

      if(junk_hdr->read_offset != junk_hdr->write_offset){

	/* This case is existing the only one element. so return the position pointing the last element. */
	junk_hdr->count--;
	junk_hdr->read_offset = junk_hdr->write_offset;
	junk_hdr->remain_size -= ( sizeof(wv_shm_junk_elem_hdr_t) + elem_hdr->size );
      }
      else{

	/* This case is the empty state, so make the read position same with the write position. */
	ret_addr_read = NULL;
      }
    }
  }

  return ret_addr_read;
}


void wv_shm_show_junk(wv_shm_junk_hdr_t* shm_junk_hdr)
{
  if (shm_junk_hdr){
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
    wv_write_log(LOG_INF, "shm_junk_hdr->quu_start_offset : %ld", shm_junk_hdr->quu_start_offset);
    wv_write_log(LOG_INF, "shm_junk_hdr->quu_end_offset : %ld", shm_junk_hdr->quu_end_offset );
    wv_write_log(LOG_INF, "shm_junk_hdr->read_offset : %ld", shm_junk_hdr->read_offset);
    wv_write_log(LOG_INF, "shm_junk_hdr->write_offset : %ld", shm_junk_hdr->write_offset);
    wv_write_log(LOG_INF, "shm_junk_hdr->prev_write_offset : %ld", shm_junk_hdr->prev_write_offset);
    wv_write_log(LOG_INF, "shm_junk_hdr->remain_size : %ld", shm_junk_hdr->remain_size);
    wv_write_log(LOG_INF, "shm_junk_hdr->count : %ld", shm_junk_hdr->count);
  }
  else{

    wv_write_log(LOG_INF, "The shared memory junk file was NULL.");
  }
}


int wv_shm_check_init(){
  if (shm_meta == NULL){
    wv_write_log(LOG_INF, "Please initailize the shared memory with wv_shm_init()...");
    return 0;
  }
  return 1;
}
