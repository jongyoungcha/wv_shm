#include <wv_shm_mngt.h>
#include <wv_log.h>
#include "test_common.h"


int dump_file(int argc, char* argv[])
{
  int ret = 0;
  char* logname = "dump_file.log";

  wv_shm_junk_hdr_t* junkhdr = NULL;
  testelem_t testelem;

  wv_init_log("./", logname);

  wv_shm_init_shm(0);

  junkhdr = wv_shm_find_junk(JUNKNAME);

  snprintf(testelem.data, 8192, "%s", "my test message!!");

  while(1)
  {
    if (wv_shm_push_elem(junkhdr, &testelem, sizeof(testelem_t)) == -1)
    {
      break;
    }
  }

  wv_shm_dump_junk(JUNKNAME, "./");
  wv_shm_clear_junk(junkhdr);

  return ret;
}
  
