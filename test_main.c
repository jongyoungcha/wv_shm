#include "wv_shm_mngt.h"

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <wv_log.h>

#define SHM_ID  7777
#define SHM_NAME "test_1"

typedef struct test_packet{
  char from_ip[256];
  int from_port;
  char data[8192];
}test_packet_t;

int wv_shm_load_test();
int wv_shm_init_test();

int main()
{
  /* wv_shm_init_test(); */

  wv_shm_load_test();
  
  return 0;
}


int wv_shm_load_test()
{
  int ret = 0;
  int index = 0;
  wv_write_log(LOG_INF, "*** Start wv_shm_load TEST ***");
  wv_init_log("./", "wv_shm_log");
  
  wv_shm_init();
  
  wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr = NULL;
  test_packet_t* ptest_packet = NULL;

  wv_shm_junk_hdr_t* shm_junk_hdr = NULL;

  shm_junk_hdr = wv_shm_find_junk("mytest_junk");
  index = wv_shm_get_junk_index(shm_junk_hdr);
  printf("index : %d", index);
  
  return ret;
}

int wv_shm_init_test()
{
  int ret = 0;
  int i=0;

  wv_init_log("./", "wv_shm_log");
  wv_write_log(LOG_INF, "*** Start wv_shm_init TEST ***");
  wv_shm_init();

  test_packet_t test_packet[5] = {
    {"111.111.111.111", 3000, "datadatadata"},
    {"222.111.111.111", 3000, "datadatadata"},
    {"333.111.111.111", 3000, "datadatadata"},
  };

  wv_write_log(LOG_INF, "Assign the shared memory junk with \"mytest_junk\"");
  wv_shm_junk_hdr_t* shm_junk_hdr = NULL;
  if ( !(shm_junk_hdr = wv_shm_assign_junk("mytest_junk")) ){
    wv_write_log(LOG_INF, "The junk memory same name was found, so we had to get the junk memory queue. ");
    shm_junk_hdr = wv_shm_find_junk("mytest_junk");
  }

  /* Write the elems */
  wv_shm_push_elem(shm_junk_hdr, &test_packet[0], sizeof(test_packet_t));
  wv_shm_push_elem(shm_junk_hdr, &test_packet[1], sizeof(test_packet_t));
  wv_shm_push_elem(shm_junk_hdr, &test_packet[2], sizeof(test_packet_t));

  wv_shm_junk_hdr_t** junk_list = (wv_shm_junk_hdr_t**) wv_shm_get_junk_list();

  for (i=0; *(junk_list+i) != NULL; i++){
    wv_shm_show_junk(*(junk_list + i));
  }

  wv_shm_free_junk_list(junk_list);

  wv_shm_dump_junk("mytest_junk", "./");

  wv_shm_show_junk(shm_junk_hdr);

  return ret;
}
