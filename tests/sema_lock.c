#include <wv_shm_mngt.h>
#include <wv_log.h>
#include "test_common.h"


int sema_lock(int argc, char* argv[])
{
  int ret = 0;
  int i = 0;

  char buf[8192] = {0};
  char* logname = "sema_lock.log";
  
  wv_init_log("./", logname);

  return ret;
}
  
