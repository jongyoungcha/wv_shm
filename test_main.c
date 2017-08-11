#include "wv_shm_mngt.h"

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define SHM_ID  7777
#define SHM_NAME "test_1"



int main(){

  wv_shm_test();

  return 0;
}
