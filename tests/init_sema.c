#include <stdio.h>
#include <wv_log.h>
#include <wv_shm_mngt.h>
#include "test_common.h"

int main(int argc, char* argv[])
{
  wv_shm_junk_hdr_t* junkhdr = NULL;

  wv_init_log("./", "init_sema.log");

  if (wv_shm_init(SHM_INIT | SEMA_INIT) == -1)
  {
    printf("wv_shm_init_shm() error...\n");
  }

  if((junkhdr = wv_shm_assign_junk(JUNKNAME)) == NULL)
  {
    printf("wv_shm_assign_junk() error...\n");
  }

  return 0;
}




