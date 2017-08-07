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


int main(){
  /* wv_file_t* wv_file; */
  wv_shm_init();

  return 0;
}
