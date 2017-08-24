#include <wv_shm_mngt.h>
#include <wv_log.h>
#include "test_common.h"


int load_file(int argc, char* argv[])
{
  int ret = 0;
  char* logname = "load_file.log";
  char pwd[8192] = {0};

  wv_shm_junk_hdr_t* junkhdr = NULL;
  wv_shm_junk_elem_hdr_t* elemhdr = NULL;
  testelem_t* testelem = NULL;
  
  wv_init_log("./", logname);
  
  wv_shm_init_shm(0);

  getcwd(pwd, 8192);

  junkhdr = wv_shm_load_junk(pwd, JUNKNAME);

  while(1)
  {
    if((elemhdr = wv_shm_peek_elem_hdr(junkhdr)))
    {
      if((testelem = wv_shm_pop_elem_data(junkhdr, elemhdr)) != NULL)
      {
	printf("%s\n", testelem->data);
      }
    }
    else
    {
      break;
    }
  }

  return ret;
}
