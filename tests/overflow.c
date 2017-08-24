#include <wv_shm_mngt.h>
#include <wv_log.h>
#include "test_common.h"

int overflow(int argc, char* argv[]){

  int ret = 0;
  int i = 0;
  
  char* logname = "overflow.log";

  wv_shm_junk_hdr_t* junkhdr = NULL;

  wv_init_log("./", logname);
  
  wv_shm_init_shm(0);

  if ((junkhdr = wv_shm_assign_junk(JUNKNAME)) == NULL)
  {
    wv_write_log(LOG_ERR, "Assigning shared memory junk was failed...");
    return ret = -1;
  }

  while(1)
  {
    testelem_t elem;
    snprintf(elem.data, 8192, "%s", "my test message!!!");
    if (wv_shm_push_elem(junkhdr, &elem, sizeof(elem)) != -1){
      
      wv_write_log(LOG_INF, "remained junk size : %ld", junkhdr->remainsz);

      if (junkhdr->remainsz < sizeof(elem))
      {
	for (i=0; i<10; i++){
	  wv_shm_push_elem(junkhdr, &elem, sizeof(elem));
	}

	if (junkhdr->remainsz > 0 && junkhdr->remainsz < sizeof(elem))
	{
	  ret = 0;
	}else{
	  wv_write_log(LOG_INF, "junk remained size was smaller than 0");	
	  ret = -1;
	}
      
	break;
      }
    }
    else
    {
      wv_write_log(LOG_ERR, "Push a element was failed...");
    }
    
  }

  return ret;
}
