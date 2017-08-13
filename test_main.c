#include "wv_shm_mngt.h"

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define SHM_ID  7777
#define SHM_NAME "test_1"

typedef struct test_packet{
  char from_ip[256];
  int from_port;
  char data[8192];
}test_packet_t;

int main()
{
  wv_shm_test();

  return 0;
}

int wv_shm_test()
{
  int ret = 0;
  wv_shm_init();

  test_packet_t test_packet[5] = {
    {"111.111.111.111", 3000, "datadatadata"},
    {"222.111.111.111", 3000, "datadatadata"},
    {"333.111.111.111", 3000, "datadatadata"},
  };

  wv_shm_junk_hdr_t* shm_junk_hdr =  wv_shm_assign_junk("mytest_junk_shm");
  printf("shm_junk_hdr->count : %ld\n", shm_junk_hdr->count);
  printf("shm_junk_hdr->start_addr : %p\n", shm_junk_hdr->quu_start_addr);
  printf("shm_junk_hdr->end_addr : %p\n", shm_junk_hdr->quu_end_addr);
  printf("shm_junk_hdr->remain_size : %ld\n", shm_junk_hdr->remain_size);
  printf("shm_junk_hdr->shm_name : %s\n", shm_junk_hdr->shm_name);
  printf("shm_junk_hdr->is_assigned : %d\n", shm_junk_hdr->is_assigned);

  wv_shm_junk_show(shm_junk_hdr);

  /* Write the elems */
  wv_shm_push_elem(shm_junk_hdr, &test_packet[0], sizeof(test_packet_t));
  wv_shm_push_elem(shm_junk_hdr, &test_packet[1], sizeof(test_packet_t));
  wv_shm_push_elem(shm_junk_hdr, &test_packet[2], sizeof(test_packet_t));

  /* Read the elems */
  wv_shm_junk_elem_hdr_t* shm_junk_elem_hdr = NULL;
  test_packet_t* ptest_packet = NULL;

  if ( (shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)wv_shm_peek_elem_hdr(shm_junk_hdr)) ){
    ptest_packet = (test_packet_t*) wv_shm_pop_elem_data(shm_junk_hdr, shm_junk_elem_hdr);
    printf("ptest_packet->from_ip : %s\n", ptest_packet->from_ip);
    printf("ptest_packet->port : %d\n",  ptest_packet->from_port);
    printf("ptest_packet->data : %s\n",  ptest_packet->data);
  }

  if ( (shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)wv_shm_peek_elem_hdr(shm_junk_hdr)) ){
    ptest_packet = (test_packet_t*) wv_shm_pop_elem_data(shm_junk_hdr, shm_junk_elem_hdr);
    printf("ptest_packet->from_ip : %s\n", ptest_packet->from_ip);
    printf("ptest_packet->port : %d\n",  ptest_packet->from_port);
    printf("ptest_packet->data : %s\n",  ptest_packet->data);
  }

  if ( (shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)wv_shm_peek_elem_hdr(shm_junk_hdr)) ){
    ptest_packet = (test_packet_t*) wv_shm_pop_elem_data(shm_junk_hdr, shm_junk_elem_hdr);
    printf("ptest_packet->from_ip : %s\n", ptest_packet->from_ip);
    printf("ptest_packet->port : %d\n",  ptest_packet->from_port);
    printf("ptest_packet->data : %s\n",  ptest_packet->data);
  }

  if ( (shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)wv_shm_peek_elem_hdr(shm_junk_hdr)) ){
    if((ptest_packet = (test_packet_t*) wv_shm_pop_elem_data(shm_junk_hdr, shm_junk_elem_hdr))){
      printf("ptest_packet->from_ip : %s\n", ptest_packet->from_ip);
      printf("ptest_packet->port : %d\n",  ptest_packet->from_port);
      printf("ptest_packet->data : %s\n",  ptest_packet->data);
    }else{
      printf("none\n");
    }
  }

  if ( (shm_junk_elem_hdr = (wv_shm_junk_elem_hdr_t*)wv_shm_peek_elem_hdr(shm_junk_hdr)) ){
    if((ptest_packet = (test_packet_t*) wv_shm_pop_elem_data(shm_junk_hdr, shm_junk_elem_hdr))){
      printf("ptest_packet->from_ip : %s\n", ptest_packet->from_ip);
      printf("ptest_packet->port : %d\n",  ptest_packet->from_port);
      printf("ptest_packet->data : %s\n",  ptest_packet->data);
    }else{
      printf("none\n");
    }
  }


  wv_shm_junk_hdr_t** junk_list = (wv_shm_junk_hdr_t**) wv_shm_get_junk_list();
  

  return ret;
}
