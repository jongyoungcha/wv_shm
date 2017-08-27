#include <wv_shm_mngt.h>
#include <wv_log.h>


int shm_init(int argc, char* argv[])
{
  int ret = 0;

  char* logname = "shm_init.log";

  wv_init_log("./", logname);
  
  wv_shm_init(SHM_INIT | SEMA_INIT);  
  
  return ret;
}
