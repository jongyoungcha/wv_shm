#include <sys/types.h> 
#include <sys/sem.h> 
#include <sys/ipc.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>

#define SEMKEY 2345 

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
};

static int  semid;

int main(int argc, char **argv)
{
    char buf[11];
    char count[11];

    union semun sem_union; 
    int sem_num = 2;

    memset(buf, 0x00, 11);
    memset(count, 0x00, 11);

    if ((semid = semget((key_t)234, 0, 0660|IPC_CREAT)) != -1){
      semctl(semid, 0, IPC_RMID);
    }

    // 세마포설정을 한다. 
    semid = semget((key_t)234, sem_num, 0660|IPC_CREAT);
    if (semid == -1)
    {
        perror("semget error ");
        exit(0);
    }    

    /* 세마포어 초기화 */
    sem_union.val = 1;
    if ( -1 == semctl( semid, 0, SETVAL, sem_union)){

      printf( "semctl()-SETVAL0 실행 오류\n");
      return -1;
    }

    if ( -1 == semctl( semid, 1, SETVAL, sem_union)){

      printf( "semctl()-SETVAL1 실행 오류\n");
      return -1;
    }
    return 1;
}
