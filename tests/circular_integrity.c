#include <wv_shm_mngt.h>
#include <wv_log.h>
#include "test_common.h"

int circular_integrity(int argc, char* argv[])
{
  int ret = 0;
  char* logname = "circular_integrity.log";
  int i = 0;

  wv_shm_junk_elem_hdr_t* elemhdr = NULL;
  wv_shm_junk_hdr_t* junkhdr = NULL;

  testelem_t* ptestelem = NULL;

  testelem_t elem;

  wv_init_log("./", logname);
  wv_shm_init_shm(0);

  if ((junkhdr = wv_shm_find_junk(JUNKNAME)) == NULL)
  {
    wv_write_log(LOG_ERR, "Getting the previous junk was failed..");
    return ret = -1;
  }

  /* wv_shm_clear_junk(junkhdr); */
  wv_shm_show_junk(junkhdr);

  snprintf(elem.data, 8192, "%s", "test message");

  /* Push the elements until the junk was full. */
  while(1)
  {
    if (wv_shm_push_elem(junkhdr, &elem, sizeof(testelem_t)) == -1)
    {
      break;
    }
  }

  /* Pop the 2 elements from the junk */
  for (i=0; i<2; i++)
  {
    if ((elemhdr = wv_shm_peek_elem_hdr(junkhdr)) != NULL)
    {
      if ((ptestelem = wv_shm_pop_elem_data(junkhdr, elemhdr)) == NULL)
      {
	wv_write_log(LOG_ERR, "when we poped the two elements from queue, the queue size was smaller than 2.");
	return ret = -1;
      }
      else
      {
	wv_write_log(LOG_INF, "Log data : %s", ptestelem->data);
      }
    }
  }

  /* Push the 2 elements to the junk */
  for (i=0; i<2; i++)
  {
    if (wv_shm_push_elem(junkhdr, &elem, sizeof(testelem_t)) == -1)
    {
      wv_write_log(LOG_ERR, "When we pushed the two elements to the queue,"
		   "The remained space of it was smaller then the push elements..." );

      return ret = -1;
    }
  }

  if (wv_shm_push_elem(junkhdr, &elem, sizeof(testelem_t)) != -1)
  {
    wv_write_log(LOG_ERR, "Push Error");
    return ret = -1;
  }

  int count = junkhdr->count;
  
  for (i=0; i<count; i++)
  {
    elemhdr = wv_shm_peek_elem_hdr(junkhdr);
    if (wv_shm_pop_elem_data(junkhdr, elemhdr) == NULL){
      wv_write_log(LOG_ERR, "The count was not matched with real count");
      return ret = -1;
    }
  }

  if (wv_shm_pop_elem_data(junkhdr, elemhdr) != NULL){
    wv_write_log(LOG_ERR, "The count was not matched with real count");
    return ret = -1;
  }
  
  return ret;
}
