#include <stdio.h>
#include <wv_log.h>
#include <wv_shm_mngt.h>
#include "test_common.h"

int main(int argc, char* argv[])
{
  int ret = 0;

  wv_shm_junk_hdr_t* junkhdr = NULL;
  int junkidx = -1;
  int seq = 0;
  int i = 0;
  int j = 0;
  char* logname = "producer.log";
  time_t starttime;
  time_t endtime;

  testelem_t elem;

  wv_init_log("./", logname);

  wv_shm_init(0);
  
  if ((junkhdr = wv_shm_find_junk(JUNKNAME)) == NULL)
  {
    fprintf(stderr, "wv_shm_find_junk() failed...");
    return -1;
  }
    
  junkidx = wv_shm_get_junk_index(junkhdr);

  time(&starttime);
  for (i = 0; i<100; i++)
  {
    //printf("producer : try locking\n");
    wv_shm_lock_quu(junkidx);

    /* getchar(); */

    for(j = 0; j < 100000; j++)
    {
      snprintf(elem.data, 8192, "test message %d", seq++);
      /* printf("pushing %s\n", elem.data); */
      wv_shm_push_elem(junkhdr, &elem, sizeof(testelem_t));
    }

    //printf("producer : try unlocking\n");
    wv_shm_unlock_quu(junkidx);
  }
  time(&endtime);

  printf("producer time span :%ld\n",  endtime - starttime);
  

  return ret;
}
