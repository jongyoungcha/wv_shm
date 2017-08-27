#include <stdio.h>
#include <wv_log.h>
#include <wv_shm_mngt.h>
#include "test_common.h"

int main(int argc, char* argv[])
{
  wv_shm_junk_hdr_t* junkhdr = NULL;
  wv_shm_junk_elem_hdr_t* elemhdr = NULL;
  
  int junkidx = -1;
  int seq = 0;
  int i = 0;
  char* logname = "consumer.log";

  testelem_t* pelem = NULL;

  wv_init_log("./", logname);

  wv_shm_init(0);
  
  if ((junkhdr = wv_shm_find_junk(JUNKNAME)) == NULL)
  {
    fprintf(stderr, "wv_shm_find_junk() failed...");
    return -1;
  }
    
  junkidx = wv_shm_get_junk_index(junkhdr);
  
  while(1)
  {
    printf("try locking\n");
    wv_shm_lock_quu(junkidx);

    for(i = 0; i < 4; i++)
    {
      if ((elemhdr = wv_shm_peek_elem_hdr(junkhdr)))
      {
      	if((pelem = wv_shm_pop_elem_data(junkhdr, elemhdr)))
      	{
      	  printf("poped message : %s\n", pelem->data);
      	}
      }
      sleep(1);
    }

    printf("try unlocking\n");
    wv_shm_unlock_quu(junkidx);
  }
  
  return 0;
}
  
