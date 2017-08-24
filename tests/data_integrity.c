#include <wv_shm_mngt.h>
#include <wv_log.h>
#include "test_common.h"


int data_integrity(int argc, char* argv[])
{
  int ret = 0;
  int i = 0;
  int push_count = 0;
  int pop_count = 0;

  char buf[8192] = {0};

  char* logname = "data_integrity.log";
  wv_shm_junk_hdr_t* junkhdr = NULL;
  wv_shm_junk_elem_hdr_t* elemhdr = NULL;
  testelem_t elem;
  testelem_t** elem_list = malloc(1000*sizeof(testelem_t*));
  testelem_t* pelem = NULL;

  memset(elem_list, 0x00, sizeof(1000*sizeof(testelem_t*)));
      
  wv_init_log("./", logname);
    
  wv_shm_init_shm(0);
  
  /* Getting Shared memory junk */
  if ((junkhdr = wv_shm_find_junk(JUNKNAME)) == NULL)
  {
    wv_write_log(LOG_ERR, "Getting the previous junk was failed...");
    return ret = -1;
  }

  wv_shm_clear_junk(junkhdr);

  /* Push the 1000 of the elem */
  for ( i = 0; i < 200; i++ )
  {
    snprintf(elem.data, 8192, "%s%d", "my test message!!", i);
    if (wv_shm_push_elem(junkhdr, &elem, sizeof(elem)) == -1)
    {
      wv_write_log(LOG_ERR, "Push failed...");
      if (junkhdr->remainsz < sizeof(elem))
      {
	wv_write_log(LOG_INF, "the junk queue was full");
	*(elem_list+i) = malloc(sizeof(testelem_t));
    	memcpy(*(elem_list+i), &elem, sizeof(testelem_t));
      }
      else{
	wv_write_log(LOG_ERR, "push failed and the size was remain.... so failed...");
	return ret = -1;
      }
    }
    else
    {
      push_count++;
    }
  }

  /* Pop the 1000 of the elem */
  for ( i = 0; i < 200; i++ )
  {
    if ( (elemhdr = wv_shm_peek_elem_hdr(junkhdr)) != NULL)
    {
      if((pelem = wv_shm_pop_elem_data(junkhdr, elemhdr)) != NULL)
      {
	snprintf(buf, 8192, "%s%d", "my test message!!", i);
	
	if(strcmp(pelem->data, buf) != 0)
	{
	  wv_write_log(LOG_ERR, "buffer message : %s", buf);
	  wv_write_log(LOG_ERR, "pelem->data : %s", pelem->data);
	  return -1;
	}

	free(pelem);
	pop_count++;
      }
    }
  }

  if (push_count != pop_count)
  {
    wv_write_log(LOG_ERR, "Push count and pop count are not same...");
    return -1;
  }

  wv_shm_show_junk(junkhdr);
  
  return ret;
}






