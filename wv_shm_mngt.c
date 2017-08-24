#include "wv_shm_mngt.h"

#define SHMMAX 5000
#define SHMMIN 1000

wv_shm_meta_t* _shmmeta = NULL;
wv_shm_quusmphr_t* _shmquusmphr = NULL;
struct shmbuf* _shmsemopen = NULL;
struct shmbuf* _shmsemclose = NULL;
static int _semid = 0;

int wv_shm_shm(int flags)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);
  
  int ret = 0;

  if ( wv_shm_init_shm(flags) < 0 )
  {
    wv_write_log(LOG_INF, "wv_shm_init_shm() failed...");
    return ret = -1;
  }

  if ( wv_shm_init_smphr(flags) < 0 )
  {
    wv_write_log(LOG_INF, "wv_shm_init_shmphr failed...");
    return ret = -1;
  }

  return ret;
}


int wv_shm_init_shm (int flags)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  int ret = 0;
  void* shmaddr = (void*)0;
  void* curraddr = NULL;

  int shmid = 0;
  int shm_exists = 0;
  
  struct shmid_ds shm_ds;

  _shmmeta = (wv_shm_meta_t*)malloc(sizeof(wv_shm_meta_t));
  memset(_shmmeta, 0x00, sizeof(wv_shm_meta_t));

  _shmmeta->count = 0;
  _shmmeta->shm_key = SHM_KEY;
  _shmmeta->shm_junk = SHM_JUNK_SIZE;
  _shmmeta->shm_totalsize = SHM_TOTAL_SIZE;

  /* When SHM_INIT enabled, remove the previous shared memory */
  if ( (shmid = shmget(_shmmeta->shm_key,_shmmeta->shm_totalsize, IPC_CREAT | IPC_EXCL)) == -1 )
  {
    if (flags && SHM_INIT)
    {
      if ((shmid = shmget(_shmmeta->shm_key,_shmmeta->shm_totalsize, 0)))
      {
	wv_write_log(LOG_INF, "SHM_INIT option existed, remove the shared memory...");
	shmctl(shmid, IPC_RMID, 0);	  
      }
      else
      {
	wv_write_log(LOG_INF, "When removed previous shared memory, getting shared memory id was failed...");
	exit(errno);
      } 
    }
  }

  /* Check if there is the previous shared memory. */
  if ( (shmid = shmget(_shmmeta->shm_key,_shmmeta->shm_totalsize, IPC_CREAT | IPC_EXCL)) == -1 )
  {
    wv_write_log(LOG_INF, "There is the previous shared memory was existing...");

    /* Getting the shared memory id by the shared memory key */
    if ( (shmid = shmget(_shmmeta->shm_key,_shmmeta->shm_totalsize, 0)) == -1 )
    {
      /* Failed Getting the shared memory id */
      wv_write_log(LOG_ERR, "Getting the previous shared memory faild... (shm_id : %d, errno : %s)",
		   shmid, strerror(errno));
      exit( errno );
    }
    else
    {
      shm_exists = 1;
    }
  }
  else
  {
    /* There is no the previous shared memory. So we shold make a new shared memory by key */
    if ( (shmid = shmget(_shmmeta->shm_key, _shmmeta->shm_totalsize, IPC_CREAT|0660)) == -1)
    {
      wv_write_log(LOG_ERR, "We failed making a new shared memory..... ");
      exit( errno );
    }
    else
    {
      shm_exists = 0;
    }
  }

  /* Attaching shared memory. */
  shmaddr = shmat(shmid, (void*)0, 0);
  if ( shmaddr == (void*)(-1) )
  {
    wv_write_log(LOG_ERR, "Getting sharedaddr : %p", _shmmeta->shm_startaddr);
    wv_write_log(LOG_ERR, "cur_addr : %p", curraddr);
    return -1;
  }

  if (shmctl(shmid, IPC_STAT, &shm_ds) == -1)
  {
    wv_write_log(LOG_ERR, "[Error] Getting shred info failed...");
    return -1;
  }

  wv_write_log(LOG_INF, "shm_ds.shm_segsz : %ld", shm_ds.shm_segsz);
  wv_write_log(LOG_INF, "shm_ds.shm_ctime : %ld", shm_ds.shm_ctime);

  if ( shm_exists )
  {

    wv_shm_load_meta(shmaddr);
  }
  else
  {
    /* Setting the shared memory meta information. */
    wv_shm_init_meta(shmaddr);
    wv_shm_sync_meta(_shmmeta);
  }

  if (wv_shm_init_smphr(flags) == -1){

    return -1;
  }
  
  return ret;
}


int wv_shm_load_meta(void* shm_start_addr)
{
  int ret = 0;
  wv_write_log(LOG_INF, "[ %s ]", __func__);

  if (!wv_shm_check_init())
  {
    return ret = -1;
  }

  if (shm_start_addr == NULL){
    return ret = -1;
  }

  _shmmeta = (wv_shm_meta_t*)wv_shm_rd(shm_start_addr, sizeof(wv_shm_meta_t), NULL);
  _shmmeta->shm_startaddr = shm_start_addr;
  _shmmeta->shm_endaddr = shm_start_addr + _shmmeta->shm_totalsize;

  wv_write_log(LOG_INF, "<The loaded shared meta data >");
  wv_write_log(LOG_INF, "* shm_meta.count : %ld", _shmmeta->count);
  wv_write_log(LOG_INF, "* shm_meta.shm_start_addr : %p", shm_start_addr);
  wv_write_log(LOG_INF, "* shm_meta.shm_end_addr : %p", shm_start_addr + _shmmeta->shm_totalsize);
  wv_write_log(LOG_INF, "* shm_meta.key : %ld", _shmmeta->shm_key);
  wv_write_log(LOG_INF, "* shm_meta.shm_junk_size : %ld", _shmmeta->shm_junk);
  wv_write_log(LOG_INF, "* shm_meta.shm_total_size : %ld", _shmmeta->shm_totalsize);

  return ret;
}

int wv_shm_init_meta(void* shm_start_addr)
{
  wv_write_log(LOG_INF, "[ %s ]", __func__);

  int ret = 0;
  int i = 0;
  size_t pagesize, shm_allocsize = 0;
  size_t curroffset = 0;

  _shmmeta->shm_startaddr = shm_start_addr;
  pagesize = getpagesize();
  shm_allocsize = (SHM_TOTAL_SIZE / pagesize) * pagesize + pagesize;
  _shmmeta->shm_endaddr = _shmmeta->shm_startaddr + shm_allocsize;
  _shmmeta->shm_totalsize = shm_allocsize;
  memset(_shmmeta->shm_startaddr, 0x00, sizeof(shm_allocsize));

  wv_write_log(LOG_INF, "Initailizing the shared memory meta structure...");
  wv_write_log(LOG_INF, "shm_meta->count : %ld", _shmmeta->count);
  wv_write_log(LOG_INF, "shm_meta->shm_start_addr : %p", _shmmeta->shm_startaddr);
  wv_write_log(LOG_INF, "shm_meta->shm_end_addr : %p", _shmmeta->shm_endaddr);
  wv_write_log(LOG_INF, "shm_meta->key : %ld", _shmmeta->shm_key);
  wv_write_log(LOG_INF, "shm_meta->shm_junk_size : %ld", _shmmeta->shm_junk);
  wv_write_log(LOG_INF, "shm_meta->shm_total_size : %ld", _shmmeta->shm_totalsize);

  /* Split the section of shared memory */
  curroffset = sizeof(wv_shm_meta_t);
  _shmmeta->count = 0;

  wv_write_log(LOG_INF, "Initailizing the shared memory junk queuere...");
  for ( i=0; i < SHM_MAX_COUNT && ((_shmmeta->shm_startaddr + curroffset) < _shmmeta->shm_endaddr); i++ ){

    wv_write_log(LOG_INF, "Initailizing shared memory junk[%d]", i);
    wv_shm_junk_init(NULL, curroffset, curroffset + SHM_JUNK_SIZE);
    _shmmeta->arr_junkhdr_offsets[i] = curroffset;
    curroffset += SHM_JUNK_SIZE;
    _shmmeta->count++;
  }

  return ret;
}


int wv_shm_init_smphr(int is_init)
{
  wv_write_log(LOG_INF, "[ %s ]", __func__);

  int ret = 0;
  int i = 0;

  wv_shm_quusmphr_t smphrunion;

  if (!_shmmeta)
  {
    wv_write_log(LOG_WRN, "Please call wv_shm_init_shm(), Before conduct this function... ");
  }
  
  smphrunion.val = _shmmeta->count;

  if (!wv_shm_check_init())
  {
    return ret = -1;
  }

  /* Remove previous the semaphores set. */
  if (is_init)
  {
    if ((_semid = semget((key_t)SHM_SMPR_KEY, 0, 0660|IPC_CREAT|IPC_EXCL)) == -1)
    {
      wv_write_log(LOG_INF, "There was the semaphore set. (perror : %s)", strerror(errno));
      semctl(_semid, 0, IPC_RMID);
    }
  }

  if ( !_shmquusmphr )
  {
    _shmquusmphr = malloc(sizeof(wv_shm_quusmphr_t));
  }

  /* Getting a semaphore set id by the key */
  if ((_semid = semget((key_t)SHM_SMPR_KEY, _shmmeta->count, 0660|IPC_CREAT)) == -1)
  {
    wv_write_log(LOG_ERR, "semget() errror (perror : %s)", strerror(errno));
    return ret = -1;
  }

  /* Initialize the semaphores */
  for (i=0; i<_shmmeta->count; i++)
  {
    _shmquusmphr->val = 1;

    if (semctl(_semid, i, SETVAL, smphrunion) == -1)
    {
      wv_write_log(LOG_ERR, "semctl() error (perror : %s)", strerror(errno));
      return ret = -1;
    }
  }

  return ret;
}


int wv_shm_lock_quu(int index){

  wv_write_log(LOG_INF, "[ %s ]", __func__);

  int ret = 0;
  struct sembuf sem_open = {index, -1, SEM_UNDO};

  if (index < 0)
  {
    wv_write_log(LOG_ERR, "the index value was smaller than 0...", index);
    return ret = -1;
  }

  if (semop(_semid, &sem_open, 1) == -1)
  {
    wv_write_log(LOG_ERR, "Locking the semaphore was failed...(perror : %s)", strerror(errno));
    return ret = -1;
  }

  return ret;
}


int wv_shm_unlock_quu(int index){

  wv_write_log(LOG_INF, "[ %s ]", __func__);

  int ret = 0;
  struct sembuf sem_close = {index, 1, SEM_UNDO};

  if (index < 0)
  {
    wv_write_log(LOG_ERR, "the index value was smaller than 0...", index);
    return ret = -1;
  }

  if (semop(_semid, &sem_close, 1) == -1)
  {
    wv_write_log(LOG_ERR, "Unlocking the semaphore was failed...(perror : %s)", strerror(errno));
    return ret = -1;
  }

  return ret;
}


int wv_shm_get_junk_count()
{
  int ret = -1;

  if ( _shmmeta )
  {
    return ret = _shmmeta->count;
  }
  else
  {
    return ret;
  }
}


int wv_shm_sync_meta()
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  int ret = 0;

  wv_write_log(LOG_INF, "<The initailized shared meta data >");
  wv_write_log(LOG_INF, "* shm_meta.count : %ld", _shmmeta->count);
  wv_write_log(LOG_INF, "* shm_meta.shm_start_addr : %p", _shmmeta->shm_startaddr);
  wv_write_log(LOG_INF, "* shm_meta.shm_end_addr : %p", _shmmeta->shm_startaddr + _shmmeta->shm_totalsize);
  wv_write_log(LOG_INF, "* shm_meta.key : %ld", _shmmeta->shm_key);
  wv_write_log(LOG_INF, "* shm_meta.shm_junk_size : %ld", _shmmeta->shm_junk);
  wv_write_log(LOG_INF, "* shm_meta.shm_total_size : %ld", _shmmeta->shm_totalsize);

  if ( _shmmeta && wv_shm_wr( 0,
			      _shmmeta,
			      sizeof(wv_shm_meta_t),
			      NULL ) == NULL ){

    wv_write_log(LOG_ERR, "When wrote the shared memory meta data to the shared memory, error occured. ");
    return ret = 1;
  }

  return ret;
}


wv_shm_junk_hdr_t* wv_shm_find_junk(const char* junk_name)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);  
  wv_shm_junk_hdr_t* ret_pjunkhdr = NULL;
  int i = 0;

  if ( _shmmeta ){

    if (_shmmeta->count > 0)
    {
      for(i = 0; i < _shmmeta->count; i++)
      {
	ret_pjunkhdr = _shmmeta->shm_startaddr + _shmmeta->arr_junkhdr_offsets[i];

	if( ret_pjunkhdr->is_assigned &&
	    strncmp(ret_pjunkhdr->shm_name, junk_name, strlen(junk_name)) == 0 )
	{
	  return ret_pjunkhdr;
	}
      }
    }
    else
    {
      wv_write_log(LOG_INF, "There wasnt any shared memory junk... ");
      return ret_pjunkhdr = NULL;
    }
  }

  return ret_pjunkhdr = NULL;
}


wv_shm_junk_hdr_t* wv_shm_assign_junk(const char* junkname)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  wv_shm_junk_hdr_t* ret_pjunkhdr = NULL;
  int i = 0;

  if ( _shmmeta && junkname ){

    wv_write_log(LOG_INF, "The count of shm_meta : %d" , _shmmeta->count);
    
    if(wv_shm_find_junk(junkname)){

      wv_write_log(LOG_WRN, "When the shared memory junk assigned, "
		   "The junk having the same name was existing.");
      ret_pjunkhdr = NULL; goto ret_wv_shm_assign_junk;
    }
    
    for ( i = 0; i < _shmmeta->count; i++ ){
      wv_write_log(LOG_INF,"shm_meta->arr_junk_hdr_offsets[0] : %ld", _shmmeta->arr_junkhdr_offsets[i]);
      wv_write_log(LOG_INF,"shm_meta->arr_junk_hdr_offsets[1] : %ld", _shmmeta->arr_junkhdr_offsets[i]);

      ret_pjunkhdr = _shmmeta->shm_startaddr + _shmmeta->arr_junkhdr_offsets[i];
      wv_write_log(LOG_INF,"shm_junk_hdr : %p", ret_pjunkhdr);
      wv_write_log(LOG_INF,"shm_junk_hdr->quu_start_offset : %ld", ret_pjunkhdr->quu_startoffs);
      wv_write_log(LOG_INF,"shm_junk_hdr->quu_end_offset : %ld", ret_pjunkhdr->quu_endoffs);

      if ( ret_pjunkhdr->is_assigned == 0 ){

      	ret_pjunkhdr->is_assigned = 1;
      	snprintf(ret_pjunkhdr->shm_name, 256, "%s", junkname);

	wv_write_log(LOG_INF, "%p", ret_pjunkhdr);
      	wv_shm_clear_junk(ret_pjunkhdr);

      	return ret_pjunkhdr;
      }
      else
      {
	wv_write_log(LOG_INF, "When we try to assign the shared memory junk,"
		     "the is_assigned attribute was 1 (it means assigned...)");
	ret_pjunkhdr = NULL;
      }
    }
  }
  else
  {
    wv_write_log(LOG_ERR, "The Ware Valley Shared Memory was not initailized..." );
  }

ret_wv_shm_assign_junk:

  return ret_pjunkhdr;
}


wv_shm_junk_hdr_t* wv_shm_unassign_junk(const char* junkname)
{
  wv_write_log(LOG_INF, "[ %s ]\n" , __func__);

  wv_shm_junk_hdr_t* ret_pjunkhdr = NULL;
  int i = 0;

  if ( _shmmeta && junkname ){

    for ( i = 0; i < _shmmeta->count; i++ ){
      ret_pjunkhdr = _shmmeta->shm_startaddr + _shmmeta->arr_junkhdr_offsets[i];
      
      if ( ret_pjunkhdr->is_assigned == 1  &&
	   strncmp(ret_pjunkhdr->shm_name, junkname, strlen(junkname)) == 0 ){

	ret_pjunkhdr->is_assigned = 0;
	strcpy(ret_pjunkhdr->shm_name, "");

	wv_shm_clear_junk(ret_pjunkhdr);

	return ret_pjunkhdr;
      }
    }
  }
  else
  {
    wv_write_log(LOG_ERR, "The _shmmeta or a junkname arg was NULL...");
  }
  return ( ret_pjunkhdr = NULL );
}


int wv_shm_get_junk_index(wv_shm_junk_hdr_t* junkhdr)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);
  
  int ret = -1;
  int i = 0;

  wv_write_log(LOG_INF, "shmmeta->count : %d", _shmmeta->count);

  if (_shmmeta && junkhdr)
  {
    if (_shmmeta->count > 0)
    {
      for (i=0; i<_shmmeta->count; i++)
      {
	if (_shmmeta->arr_junkhdr_offsets[i] == junkhdr->startoffs)
	{
	  wv_write_log(LOG_INF, "find index : %ld", i);
	  return ret = i;
	}
      }
    }
  }
  else
  {
    wv_write_log(LOG_ERR, "The _shmmeta or a junkhdr arg was NULL...");
    return ret;
  }

  return ret;
}


wv_shm_junk_hdr_t** wv_shm_get_junk_list(){

  wv_shm_junk_hdr_t** ret_pjunkhdr_list = NULL;
  wv_shm_junk_hdr_t* pjunkhdr = NULL;
  int i = 0;

  if (!wv_shm_check_init()){

    return NULL;
  }

  ret_pjunkhdr_list = (wv_shm_junk_hdr_t**)malloc(sizeof(wv_shm_junk_hdr_t*) * (_shmmeta->count + 1));
  memset(ret_pjunkhdr_list, 0x00, sizeof(wv_shm_junk_hdr_t*) * (_shmmeta->count + 1));

  for(i = 0; i<_shmmeta->count; i++){
    pjunkhdr = _shmmeta->shm_startaddr + _shmmeta->arr_junkhdr_offsets[i];

    (*(ret_pjunkhdr_list+i)) = (wv_shm_junk_hdr_t*)malloc(sizeof(wv_shm_junk_hdr_t));
    snprintf((*(ret_pjunkhdr_list+i))->shm_name, 256, "%s", pjunkhdr->shm_name);
    (*(ret_pjunkhdr_list+i))->count = pjunkhdr->count;
    (*(ret_pjunkhdr_list+i))->is_assigned = pjunkhdr->is_assigned;
    (*(ret_pjunkhdr_list+i))->prev_writeoffs = pjunkhdr->prev_writeoffs;
    (*(ret_pjunkhdr_list+i))->quu_startoffs = pjunkhdr->quu_startoffs;
    (*(ret_pjunkhdr_list+i))->quu_endoffs = pjunkhdr->quu_endoffs;
    (*(ret_pjunkhdr_list+i))->readoffs = pjunkhdr->readoffs;
    (*(ret_pjunkhdr_list+i))->remainsz = pjunkhdr->remainsz;
    (*(ret_pjunkhdr_list+i))->startoffs = pjunkhdr->startoffs;
    (*(ret_pjunkhdr_list+i))->writeoffs = pjunkhdr->writeoffs;
  }

  (*(ret_pjunkhdr_list + (_shmmeta->count+1))) = NULL;

  return ret_pjunkhdr_list;
}


void wv_shm_free_junk_list(wv_shm_junk_hdr_t** junkhdr_list){

  int i = 0;

  if ( junkhdr_list ){

    for (i = 0; junkhdr_list[i]; i++){

      if(junkhdr_list[i]){

	free(junkhdr_list[i]);
      }
    }
  }

  if (junkhdr_list){
    free(junkhdr_list);
  }
}


int wv_shm_clear_junk(wv_shm_junk_hdr_t* junkhdr)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  int ret = 0;
  int junkidx = -1;

  if ( !wv_shm_check_init() )
  {
    ret = -1;

    return ret;
  }


  if ( junkhdr )
  {
    if ( junkhdr &&
	 junkhdr->quu_startoffs &&
	 junkhdr->quu_endoffs )
    {
      if ((junkidx = wv_shm_get_junk_index(junkhdr)) == -1)
      {
	wv_write_log(LOG_ERR,"Couldnt find the inext of the shared memory junk");
	return ret = -1;
      }

      memset( (_shmmeta->shm_startaddr + junkhdr->quu_startoffs),
	      0x00,
	      junkhdr->quu_endoffs - junkhdr->quu_startoffs );

      junkhdr->writeoffs = junkhdr->quu_startoffs;
      junkhdr->readoffs = junkhdr->quu_startoffs;
      junkhdr->prev_writeoffs = 0;
      junkhdr->remainsz =  junkhdr->quu_endoffs - junkhdr->quu_startoffs;
      junkhdr->count = 0;

      wv_write_log(LOG_INF,"shm_junk_hdr : %p", junkhdr);
      wv_write_log(LOG_INF,"shm_junk_hdr->quu_start_offset : %ld", junkhdr->quu_startoffs);
      wv_write_log(LOG_INF,"shm_junk_hdr->quu_end_offset : %ld", junkhdr->quu_endoffs);
    }
    else
    {
      wv_write_log(LOG_INF, "When cleared the shared memory junk, shm_junk_hdr was NULL...\n");
      ret = -1;

      return ret;
    }
  }
  
  return ret;
}


int wv_shm_dump_junk(const char* junkname, const char* dirname)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  int ret = 0;
  int wr_bufsz = 8192;
  char fullpath[8192] = {0};
  void* curpos = NULL;
  FILE* pfile = NULL;
  const char* dlmt = "/";
  wv_shm_junk_hdr_t* junkhdr = NULL;
  int junkidx = -1;
  int nwrite = 0;
  int dumpseq = 0;
  int mgcnt = 1000;
  
  /* Check the directory exsited */
  if (access(dirname, F_OK) != 0)
  {
    wv_write_log(LOG_ERR, "The directory not existed..." );
    ret = -1; goto wv_shm_dump_junk_ret;
  }
  
  /* Find if there is shm_junk existing or not. */
  if ( (junkhdr = wv_shm_find_junk(junkname)) == NULL )
  {
    wv_write_log(LOG_WRN, "Counldnt find the shared memory junk having same name..." );
    ret = -1; goto wv_shm_dump_junk_ret;
  }

  /* Getting the index of the shared memory */
  if ((junkidx = wv_shm_get_junk_index(junkhdr)  == -1))
  {
    wv_write_log(LOG_WRN, "Couldnt find the index of the shared memory junk");
    ret = -1; goto wv_shm_dump_junk_ret;    
  }


  /* Make the path to write the shared memory dump.  */
  /* If the same name of path existed, Add a sequance to the file name. */
  snprintf( fullpath, 8192, "%s%s%s.sdmp", dirname, dlmt, junkname );
  for (dumpseq=0;
       (access(fullpath, F_OK) == 0) && dumpseq < mgcnt;
       dumpseq++)
  {
    wv_write_log(LOG_INF, "Previous the shared memory dump file "
		 "having same path existed. (file path : %s)", fullpath );

    snprintf(fullpath, 8192, "%s%s%s_%d.sdmp", dirname, dlmt, junkname, dumpseq);
  }

  /* Dump the founded shm_junk to a file. */
  if ( (pfile = fopen(fullpath, "wb")) == NULL )
  {
    wv_write_log(LOG_INF, "Couldnt open the file with wb mode..." );
    ret = -1; goto wv_shm_dump_junk_ret;
  }
  else
  {
    /* Dump the founed shm_junk to a file. */
    if ( junkhdr->startoffs )
    {
      curpos = _shmmeta->shm_startaddr+ junkhdr->startoffs;

      while( 1 )
      {
	/* wv_write_log(LOG_INF, "write start addr : %p...", curpos); */
	if ((curpos + wr_bufsz) > _shmmeta->shm_startaddr + junkhdr->quu_endoffs)
	{
	  nwrite = fwrite(curpos, sizeof(char),
			  (_shmmeta->shm_startaddr + junkhdr->quu_endoffs) - curpos,
			  pfile);
	  break;
	}
	else
	{
	  nwrite = fwrite(curpos, sizeof(char), wr_bufsz, pfile);
	}
	curpos += nwrite;
      }
    }
    else
    {
      wv_write_log(LOG_INF, "the offset of the junk_hdr was 0..." );
      ret = -1; goto wv_shm_dump_junk_ret;
    }
  }

  if (pfile){
    fclose(pfile);
  }

wv_shm_dump_junk_ret:

  return ret;
}


wv_shm_junk_hdr_t* wv_shm_load_junk(const char* dirname, const char* junkname)
{
  wv_write_log(LOG_INF, "[ %s ]", __func__);

  int ret = 0;
  int i = 0;
  char fullpath[8192] = {0};

  char* dlmt = "/";
  FILE* pfile = NULL;
  size_t filesz = 0;
  int idx_assigned = -1;

  DIR* pdir = NULL;
  struct dirent* pentry = NULL;
  char* ext = NULL;

  void *startaddr = NULL;
  void *endaddr = NULL;
  void *curraddr = NULL;

  int nread = 0;

  wv_shm_junk_hdr_t* ret_pjunkhdr = NULL;
  int junkidx = -1;

  if (_shmmeta)
  {
    /* Check if there is file existing. */
    snprintf(fullpath, 8192, "%s%s%s.sdmp", dirname, dlmt, junkname);

    pdir = opendir(dirname);

    if(pdir)
    {
      while((pentry = readdir(pdir)))
      {
	ext = strrchr(pentry->d_name, '.');

	if ( ext && strncmp(pentry->d_name, ext, strlen(pentry->d_name)))
	{
	  if ( strncmp(ext, ".sdmp", strlen(".sdmp")) == 0 )
	  {
	    if ( strncmp(pentry->d_name, junkname, strlen(junkname)) == 0 )
	    {
	      wv_write_log(LOG_ERR, "We the junk file having the same name. (pentry->d_name : %s)", pentry->d_name);
	      snprintf(fullpath, 8192, "%s%s%s", dirname, dlmt, pentry->d_name);

	      break;
	    }
	  }
	}
	else
	{
	  wv_write_log(LOG_ERR, "Not have ext...(%s)", pentry->d_name);
	}
	sprintf(fullpath, "");
      }

      if (pdir)
      {
	closedir(pdir);
      }
    }
    else
    {
      wv_write_log(LOG_ERR, "There is no the directory you want... (dir_path : %s)", dirname);
      ret = -1; goto ret_wv_shm_load_junk;
    }

    /* Open the dump file. */
    if ((pfile = fopen(fullpath, "rb")) == NULL)
    {
      wv_write_log(LOG_ERR, "Getting the shared memory junk file matching with the junk_name(%s)"
		   "was failed...(file_path : %s)", junkname, fullpath);

      ret = -1; goto ret_wv_shm_load_junk;
    }

    /* Check if the size of the dump file is larger then the reserved junk size. */
    filesz = fseek(pfile, 0, SEEK_END);
    if (_shmmeta->shm_junk < filesz)
    {
      wv_write_log(LOG_ERR, "Opening the file was failed... (file_path : %s)", fullpath);

      if (pfile)
      {
	fclose(pfile);
      }
      ret = -1; goto ret_wv_shm_load_junk;
    }

    fseek(pfile, 0, SEEK_SET);

    /* Checking if There is the same name of the shared memory junk */
    for ( i=0; i<_shmmeta->count; i++)
    {
      ret_pjunkhdr = _shmmeta->shm_startaddr + _shmmeta->arr_junkhdr_offsets[i];

      if ( strncmp(ret_pjunkhdr->shm_name, junkname, strlen(junkname)) == 0 )
      {
	wv_write_log(LOG_WRN, "There is a shared memory junk having same name (file_name : %s)", junkname);

	endaddr = _shmmeta->shm_startaddr + ret_pjunkhdr->quu_endoffs;
	startaddr = _shmmeta->shm_startaddr + ret_pjunkhdr->startoffs;
	curraddr = startaddr;

	break;
      }

      if ( ret_pjunkhdr )
      {
	ret_pjunkhdr = NULL;
      }
    }

    if (ret_pjunkhdr == NULL)
    {
      /* If a pjunk_hdr were NULL, There was no the shared memory having same name. */
      /* So we should have get a unassigned shared memory and assign it with the getted junk file. */
      for ( i=0; i<_shmmeta->count; i++ ){

	ret_pjunkhdr = _shmmeta->shm_startaddr + _shmmeta->arr_junkhdr_offsets[i];
	
	if ( ret_pjunkhdr->is_assigned == 0 ){

	  wv_write_log(LOG_INF, "We found the unassigned shared memory junk (index : %d)", i);
	  idx_assigned = i;

	  ret_pjunkhdr->is_assigned = 1;
	  /* wv_shm_clear_junk(ret_pjunk_hdr); */

	  snprintf(ret_pjunkhdr->shm_name, 256, "%s", junkname);

	  wv_write_log(LOG_INF, "Start address of queue : %p", _shmmeta->shm_startaddr);
	  wv_write_log(LOG_INF, "Start address of queue : %p", _shmmeta->shm_startaddr);

	  endaddr = _shmmeta->shm_startaddr + ret_pjunkhdr->quu_endoffs;
	  startaddr = _shmmeta->shm_startaddr + ret_pjunkhdr->startoffs;
	  curraddr = startaddr;

	  break;
	}

	ret_pjunkhdr = NULL;
      }
    }

    /* Load the dump file to the main memory */
    if (ret_pjunkhdr){

      /* wv_shm_clear_junk(ret_pjunk_hdr); */

      junkidx =  wv_shm_get_junk_index(ret_pjunkhdr);

      wv_write_log(LOG_INF, "Start address of queue : %p", curraddr);

      if(pfile){

	while((nread = fread(curraddr, sizeof(char), 8192, pfile)) > 0){
	  curraddr += nread;
	}
      }
      else{
	wv_write_log(LOG_WRN, "pfile_dump is NULL.");
      }
    }
    else{

      wv_write_log(LOG_WRN, "We coulnt the available shared memory junk.");
    }
  }

ret_wv_shm_load_junk:
  return ret_pjunkhdr;
}


int wv_shm_junk_init(char* junkname, size_t startoffs, size_t endoffs)
{
  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  int ret = 0;

  int usablesz = endoffs - startoffs;

  wv_shm_junk_hdr_t junkhdr;
  wv_shm_junk_hdr_t* pjunkhdr = NULL;

  memset(&junkhdr, 0x00, sizeof(wv_shm_junk_hdr_t));

  if ( usablesz < sizeof(wv_shm_junk_hdr_t) )
  {
    ret = -1;
    wv_write_log(LOG_ERR, "When write shared memory junk header to shared memory, "
		 "the structure was bigger than the available size...");
    goto wv_shm_junk_init_ret;
  }

  /* Write the junk header information to shared memory */
  junkhdr.startoffs = startoffs;
  junkhdr.quu_startoffs = startoffs + sizeof(wv_shm_junk_hdr_t);
  junkhdr.quu_endoffs = endoffs;
  junkhdr.writeoffs = junkhdr.readoffs = startoffs;
  junkhdr.prev_writeoffs = 0;
  junkhdr.remainsz = endoffs - (startoffs + sizeof(wv_shm_junk_hdr_t));
  junkhdr.count = 0;

  if (wv_shm_wr(startoffs, &junkhdr, sizeof(wv_shm_junk_hdr_t), NULL) == NULL){

    ret = -1;
    wv_write_log(LOG_ERR, "Writing the shared memory junk was failed...");
    goto wv_shm_junk_init_ret;
  }

  pjunkhdr = _shmmeta->shm_startaddr + startoffs;

  wv_write_log(LOG_INF, "addr of shm_junk : %p", _shmmeta->shm_startaddr + pjunkhdr->startoffs);
  wv_write_log(LOG_INF, "junk_hdr->shm_name : %s", pjunkhdr->shm_name);
  wv_write_log(LOG_INF, "junk_hdr->is_assigned : %d", pjunkhdr->is_assigned);
  wv_write_log(LOG_INF, "junk_hdr->start_offset : %ld", pjunkhdr->startoffs);
  wv_write_log(LOG_INF, "junk_hdr->quu_start_offset : %ld", pjunkhdr->quu_startoffs);
  wv_write_log(LOG_INF, "junk_hdr->quu_end_offset : %ld", pjunkhdr->quu_endoffs );
  wv_write_log(LOG_INF, "junk_hdr->write_offset : %ld", pjunkhdr->writeoffs);
  wv_write_log(LOG_INF, "junk_hdr->prev_write_offset : %ld", pjunkhdr->prev_writeoffs);
  wv_write_log(LOG_INF, "junk_hdr->remain_size : %ld", pjunkhdr->remainsz);
  wv_write_log(LOG_INF, "junk_hdr->count : %ld", pjunkhdr->count);

wv_shm_junk_init_ret:

  return ret;
}


void* wv_shm_wr(size_t strtoffs, void* data, size_t size, size_t *nextoffs)
{
  void* ret_wraddr = NULL;
  void* mcpyaddr = NULL;

  wv_write_log(LOG_INF, "[ %s ]", __func__);

  if ( _shmmeta && _shmmeta->shm_startaddr ){

    ret_wraddr = _shmmeta->shm_startaddr + strtoffs;
    if ( ret_wraddr > _shmmeta->shm_endaddr ){

      wv_write_log(LOG_ERR, "The address to write was higher than end address.");
      ret_wraddr = NULL;
      goto wv_shm_wr_elem_ret;
    }

    if ( ret_wraddr + size > _shmmeta->shm_endaddr )
    {

      wv_write_log(LOG_ERR, "The address after write was higher than end address.");
      ret_wraddr = NULL;
      goto wv_shm_wr_elem_ret;
    }

    if ( (mcpyaddr = memcpy(ret_wraddr, data, size)) ) {

      if ( ret_wraddr != mcpyaddr ) {
	wv_write_log(LOG_WRN, "The address returned by memcpy was not equaled with write_addr.");
      }

      if (nextoffs) {
	*nextoffs = strtoffs + size;
      }
    }
    else{
      fprintf(stderr, "memset() was failed.... (errono : %s)\n", strerror(errno));
    } 
  }

wv_shm_wr_elem_ret:

  return ret_wraddr;
}



int wv_shm_push_elem(wv_shm_junk_hdr_t* junkhdr, void* data, size_t size)
{
  int ret = -1;
  void* allocaddr = NULL;
  wv_shm_junk_elem_hdr_t elemhdr;
  void* wraddr = NULL;
  size_t offs_afterwrite = 0;
  int junkidx = 0;
  int elemsz = 0;

  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  if (_shmmeta == NULL)
  {
    wv_write_log(LOG_INF, "Please initailize the shared memory with wv_shm_init()...");
    goto wv_shm_wr_elem_ret;
  }

  wv_write_log(LOG_INF, "shm_junk_hdr_address : %p", junkhdr);
  wv_write_log(LOG_INF, "shm_junk_hdr->startoffs : %ld", junkhdr->startoffs);
  wv_write_log(LOG_INF, "shm_junk_hdr->quu_endoffs : %ld", junkhdr->quu_endoffs);
  wv_write_log(LOG_INF, "shm_junk_hdr->writeoffs : %ld", junkhdr->writeoffs);
  wv_write_log(LOG_INF, "shm_junk_hdr->readoffs : %ld", junkhdr->readoffs);
  wv_write_log(LOG_INF, "shm_junk_hdr->remainsz : %ld", junkhdr->remainsz);
  
  if (junkhdr &&
      junkhdr->writeoffs &&
      junkhdr->quu_startoffs &&
      junkhdr->quu_endoffs)
  {
    if ((junkidx = wv_shm_get_junk_index(junkhdr)) == -1)
    {
      wv_write_log(LOG_ERR, "Couldnt found the index of the junk header.");
      goto wv_shm_wr_elem_ret;
    }

    elemsz = sizeof(wv_shm_junk_elem_hdr_t) + size;

    if ( junkhdr->remainsz < elemsz )
    {
      wv_write_log(LOG_INF, "Remaining shared memory size was smaller than an element...");
      goto wv_shm_wr_elem_ret;
    }

    if ( junkhdr->writeoffs < junkhdr->quu_startoffs )
    {
      wv_write_log(LOG_ERR, "write area was lower than end address..");
      goto wv_shm_wr_elem_ret;
    }

    if ( junkhdr->writeoffs + elemsz > junkhdr->quu_endoffs )
    {
      wv_write_log(LOG_INF, "This is section to go quu_start");
      if(junkhdr->quu_startoffs != junkhdr->readoffs &&
	 elemsz < (junkhdr->readoffs - junkhdr->quu_startoffs))
      {
	if (junkhdr->readoffs == junkhdr->writeoffs)
	{
	  junkhdr->readoffs = junkhdr->quu_startoffs;
	}

	junkhdr->writeoffs = junkhdr->quu_startoffs;
      }
      else
      {
	wv_write_log(LOG_INF, "Try write the elem to the last space, But the queue was full...");
	goto wv_shm_wr_elem_ret;
      }
    }

    /* Check the write address exceed junk_memory or not and make it coreccted. */
    /* +-------------------------+        +-------------------------+ */
    /* |                   elem_data  =>  |elem_data                | */
    /* +-------------------------+        +-------------------------+ */
    offs_afterwrite = junkhdr->writeoffs + elemsz;
    if ( offs_afterwrite > junkhdr->quu_endoffs )
    {
      junkhdr->writeoffs = junkhdr->quu_startoffs;
    }

    /* Links the two elements if the prev_write_addr exists. */
    if ( junkhdr->prev_writeoffs )
    {
      wv_shm_link_elems(junkhdr, junkhdr->prev_writeoffs, junkhdr->writeoffs);
    }

    wv_write_log(LOG_INF, "shm_meta->shm_start_addr : %p", _shmmeta->shm_startaddr);
    wv_write_log(LOG_INF, "shm_junk_hdr->write_offset : %ld", junkhdr->writeoffs);

    junkhdr->prev_writeoffs = junkhdr->writeoffs;

    /* Write the element header information */
    memset(&elemhdr, 0x00, sizeof(wv_shm_junk_elem_hdr_t));
    elemhdr.size = size;
    elemhdr.nextoffs = 0;
    wv_write_log(LOG_INF, "new elem_hdr.size : %ld", elemhdr.size);
    wv_write_log(LOG_INF, "new elem_hdr.next_offset : %ld", elemhdr.nextoffs);

    wraddr = _shmmeta->shm_startaddr + junkhdr->writeoffs;
    wv_write_log(LOG_INF, "write position : %p", wraddr);

    if ((allocaddr = memcpy(wraddr, &elemhdr, sizeof(wv_shm_junk_elem_hdr_t))))
    {
      if ( allocaddr != _shmmeta->shm_startaddr + junkhdr->writeoffs )
      {
	wv_write_log(LOG_ERR,
		     "The returned value of malloc() and head->write_addr was not equal.."
		     "(alloc_addr : %p <--> write_addr : %p)",
		     allocaddr, wraddr);

	goto wv_shm_wr_elem_ret;
      }

      /* Move the write position of junk_header to the postion added header size. */
      wv_write_log(LOG_INF, "pos_write_hdr : %p (write_offset : %ld)",
		   wraddr, junkhdr->writeoffs);

      junkhdr->writeoffs += sizeof(wv_shm_junk_elem_hdr_t);
    }
    else
    {
      wv_write_log(LOG_ERR, "memcpy() was failed when write a elem_header.");
      goto wv_shm_wr_elem_ret;
    }

    /* Write the element data information */
    wraddr = _shmmeta->shm_startaddr + junkhdr->writeoffs;
    if ((allocaddr = memcpy(wraddr, data, size)))
    {
      // debug message
      if ( allocaddr != wraddr )
      {
	wv_write_log(LOG_ERR,
		     "The returned value of malloc() and head->write_addr was not equal..\
                 (alloc_addr : %p <--> write_addr : %p)",
		     allocaddr, wraddr);

	goto wv_shm_wr_elem_ret;
      }

      wv_write_log(LOG_INF, "pos_write_data : %p (write_offset : %ld)",
		   wraddr, junkhdr->writeoffs);

      junkhdr->writeoffs = junkhdr->writeoffs + size;
      junkhdr->remainsz -= sizeof(wv_shm_junk_elem_hdr_t) + size;
      junkhdr->count++;

      /* All Step Successed */
      ret = 0;
    }
    else
    {
      junkhdr->writeoffs -= sizeof(wv_shm_junk_elem_hdr_t);
      wv_write_log(LOG_ERR, "memcpy() was failed when write a elem_data.");
      goto wv_shm_wr_elem_ret;
    }
  }
  else
  {
    wv_write_log(LOG_ERR, "The argument shm_junk_hdr was NULL.");
  }

wv_shm_wr_elem_ret:

  return ret;
}


/* Links the two elements */
int wv_shm_link_elems(wv_shm_junk_hdr_t* junkhdr, size_t prevoffs, size_t curoffs)
{
  int ret = 0;

  wv_write_log(LOG_INF, "[ %s ]" , __func__);

  wv_shm_junk_elem_hdr_t* elemhdr = NULL;

  if (prevoffs && curoffs)
  {
    elemhdr = _shmmeta->shm_startaddr + prevoffs;
    elemhdr->nextoffs = curoffs;
    wv_write_log(LOG_INF, "shm_junk_elem_hdr->next_offset : %ld" , elemhdr->nextoffs);
  }
  else
  {
    ret = -1;
  }

  return ret;
}


void* wv_shm_rd(void* startaddr, size_t size, void** nextaddr){

  void* ret = NULL;

  if ( startaddr )
  {
    if ( nextaddr ) { *nextaddr = startaddr + size; }
    
    ret = startaddr;
  }

  return ret;
}


wv_shm_junk_elem_hdr_t* wv_shm_peek_elem_hdr(wv_shm_junk_hdr_t* junkhdr)
{
  wv_write_log(LOG_INF, "[ %s ]", __func__);

  wv_shm_junk_elem_hdr_t* ret_elemhdr = NULL;
  void* readaddr = NULL;
  size_t readoffs = 0;

  if (junkhdr &&
      junkhdr->quu_startoffs &&
      junkhdr->quu_endoffs &&
      junkhdr->readoffs)
  {
    if (junkhdr->count > 0)
    {
      readoffs = junkhdr->readoffs;
      readaddr = _shmmeta->shm_startaddr + readoffs;

      wv_write_log(LOG_INF, "offset to read : %ld", readoffs);
      wv_write_log(LOG_INF, "address to read :  %p", readaddr);

      ret_elemhdr = (wv_shm_junk_elem_hdr_t*)readaddr;
      wv_write_log(LOG_INF, "ret_elem_hdr->size : %ld", ret_elemhdr->size);
      wv_write_log(LOG_INF, "ret_elem_hdr->next_offset : %ld", ret_elemhdr->nextoffs);
    }
    else
    {
      wv_write_log(LOG_INF, "The queue was empty.");
      return NULL;
    }
  }
  else
  {
    wv_write_log(LOG_ERR, "A shm_hdr or its attrs is null...");
  }

  return ret_elemhdr;
}


/* Return the read addres and add a size value to the read address */
void* wv_shm_peek_elem_data(wv_shm_junk_hdr_t* junkhdr)
{
  wv_write_log(LOG_INF, "[ %s ]", __func__);

  wv_shm_junk_elem_hdr_t* ret_elemhdr = NULL;
  void* rd_addr = NULL;
  size_t rd_offs = 0;

  if ( junkhdr &&
       junkhdr->quu_startoffs &&
       junkhdr->quu_endoffs &&
       junkhdr->readoffs )
  {

    if (junkhdr->count > 0){

      rd_offs = junkhdr->readoffs + sizeof(wv_shm_junk_elem_hdr_t);
      rd_addr = _shmmeta->shm_startaddr + rd_offs;

      wv_write_log(LOG_INF, "offset to read : %ld", rd_offs);
      wv_write_log(LOG_INF, "address to read : %p", rd_addr);

      ret_elemhdr = (wv_shm_junk_elem_hdr_t*)rd_addr;
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

  return ret_elemhdr;
}


void* wv_shm_pop_elem_data(wv_shm_junk_hdr_t* junkhdr, wv_shm_junk_elem_hdr_t* elemhdr)
{
  void* readaddr = 0;
  wv_shm_junk_elem_hdr_t* read_elemhdr = NULL;
  size_t readoffs = 0;
  int junkidx = -1;
  void* ret_elemdata = NULL;

  wv_write_log(LOG_INF,"[ %s ]" , __func__);

  if ( junkhdr &&
       junkhdr->quu_startoffs &&
       junkhdr->quu_endoffs &&
       junkhdr->readoffs &&
       elemhdr )
  {
    junkidx = wv_shm_get_junk_index(junkhdr);
    
    if(junkhdr->count == 0)
    {
      wv_write_log(LOG_INF, "The queue was empty...\n");
      return NULL;
    }

    /* Read the header information */
    wv_write_log(LOG_INF, "cur_elem_hdr address : %p", _shmmeta->shm_startaddr + junkhdr->readoffs);

    read_elemhdr = _shmmeta->shm_startaddr + junkhdr->readoffs;
    
    wv_write_log(LOG_INF, "header offset : %ld", junkhdr->readoffs);
    wv_write_log(LOG_INF, "next_offset : %ld", read_elemhdr->nextoffs);
    wv_write_log(LOG_INF, "size : %ld", read_elemhdr->size);

    if (read_elemhdr->nextoffs && elemhdr->nextoffs )
    {
      if ( read_elemhdr->nextoffs != elemhdr->nextoffs )
      {
	wv_write_log(LOG_ERR,
		     "The pos of a argument hdr was different with the read pos of current header..."
		     "(next_offset_stored : %ld <--> next_offset_wanted : %ld).",
		     read_elemhdr->nextoffs, elemhdr->nextoffs);

	return NULL;
      }
    }

    if ( read_elemhdr->size != elemhdr->size )
    {
      wv_write_log(LOG_ERR,
		   "The size of a argument hdr was different with the read size of current header..."
		   "(size_stored : %ld <--> size_wanted : %ld).",
		   read_elemhdr->size, elemhdr->size);

      return NULL;
    }

    readoffs = junkhdr->readoffs + sizeof(wv_shm_junk_elem_hdr_t);
    readaddr = _shmmeta->shm_startaddr + readoffs;

    ret_elemdata = malloc(read_elemhdr->size);
    memset(ret_elemdata, 0x00, read_elemhdr->size);
    memcpy(ret_elemdata, readaddr, read_elemhdr->size);

    wv_write_log(LOG_INF, "offset to read : %ld", readoffs);
    wv_write_log(LOG_INF, "address to read : %p", readaddr);

    if ( read_elemhdr->nextoffs )
    {
      /* Set the read postion to the next element position. */
      
      wv_write_log(LOG_INF, "Next offs was not NULL", readaddr);
      wv_write_log(LOG_INF, "memset start address read_elemhdr: %p", read_elemhdr);
      
      junkhdr->readoffs = read_elemhdr->nextoffs;
    }
    else
    {
      wv_write_log(LOG_INF, "Next offs was NULL");
      
      /* This case is existing the only one element. so return the position pointing the last element. */
      wv_write_log(LOG_INF, "memset start address read_elemhdr: %p", read_elemhdr);

      wv_shm_junk_elem_hdr_t* pelemhdr = NULL;

      if (junkhdr->readoffs >= junkhdr->writeoffs)
      {
	pelemhdr = _shmmeta->shm_startaddr + junkhdr->quu_startoffs;

	if(pelemhdr->nextoffs==0)
	{
	  wv_write_log(LOG_INF, "0");
	  if (ret_elemdata)
	  {
	    free(ret_elemdata);
	  }
	  
	  return NULL;
	}
	else
	{
	  wv_write_log(LOG_INF, "1");
	  junkhdr->readoffs = junkhdr->quu_startoffs;
	}
      }
      else
      {
	wv_write_log(LOG_INF, "Next offs was NULL");
	junkhdr->readoffs = junkhdr->writeoffs;
      }
    } 
  }
  
  junkhdr->remainsz += ( sizeof(wv_shm_junk_elem_hdr_t) + elemhdr->size );
  junkhdr->count--;

  return ret_elemdata;
}


void wv_shm_show_junk(wv_shm_junk_hdr_t* junkhdr)
{
  if (junkhdr){
    wv_write_log(LOG_INF, "[ %s ]" , __func__);
    printf("shm_junk_hdr->count : %ld\n", junkhdr->count);
    printf("shm_junk_hdr->start_offset : %ld\n", junkhdr->quu_startoffs);
    printf("shm_junk_hdr->end_offset : %ld\n", junkhdr->quu_endoffs);
    printf("shm_junk_hdr->remain_size : %ld\n", junkhdr->remainsz);
    printf("shm_junk_hdr->shm_name : %s\n", junkhdr->shm_name);
    printf("shm_junk_hdr->is_assigned : %d\n", junkhdr->is_assigned);

    wv_write_log(LOG_INF, "<junk_header : %p>", junkhdr);
    wv_write_log(LOG_INF, "shm_junk_hdr->shm_name : %s", junkhdr->shm_name);
    wv_write_log(LOG_INF, "shm_junk_hdr->is_assigned : %d", junkhdr->is_assigned);
    wv_write_log(LOG_INF, "shm_junk_hdr->start_offset : %ld", junkhdr->startoffs);
    wv_write_log(LOG_INF, "shm_junk_hdr->quu_start_offset : %ld", junkhdr->quu_startoffs);
    wv_write_log(LOG_INF, "shm_junk_hdr->quu_end_offset : %ld", junkhdr->quu_endoffs );
    wv_write_log(LOG_INF, "shm_junk_hdr->read_offset : %ld", junkhdr->readoffs);
    wv_write_log(LOG_INF, "shm_junk_hdr->write_offset : %ld", junkhdr->writeoffs);
    wv_write_log(LOG_INF, "shm_junk_hdr->prev_write_offset : %ld", junkhdr->prev_writeoffs);
    wv_write_log(LOG_INF, "shm_junk_hdr->remain_size : %ld", junkhdr->remainsz);
    wv_write_log(LOG_INF, "shm_junk_hdr->count : %ld", junkhdr->count);
  }
  else{

    wv_write_log(LOG_INF, "The shared memory junk file was NULL.");
  }
}


int wv_shm_check_init()
{
  if (_shmmeta == NULL){
    wv_write_log(LOG_INF, "Please initailize the shared memory with wv_shm_init()...");
    return 0;
  }
  return 1;
}
