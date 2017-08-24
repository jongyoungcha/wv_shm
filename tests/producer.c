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
  char* logname = "producer.log";

  testelem_t elem;

  wv_init_log("./", logname);

  wv_shm_init_shm(0);
  
  if ((junkhdr = wv_shm_find_junk(JUNKNAME)) == NULL)
  {
    fprintf(stderr, "wv_shm_find_junk() failed...");
    return -1;
  }
    
  junkidx = wv_shm_get_junk_index(junkhdr);
  
  while(1)
  {
    printf("locking\n");
    wv_shm_lock_quu(junkidx);

    for(i = 0; i < 4; i++)
    {
      snprintf(elem.data, 8192, "test message %d", seq++);
      printf("pushing %s\n", elem.data);
      wv_shm_push_elem(junkhdr, &elem, sizeof(testelem_t));
      sleep(2);
    }

    printf("unlocking\n");
    wv_shm_unlock_quu(junkidx);
  }

  return ret;
}
