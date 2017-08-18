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
    FILE* fp;
    char buf[11];
    char count[11];

    // open 과 close 를 위한 sembuf 구조체를 정의한다. 
    struct sembuf mysem_open  = {0, -1, SEM_UNDO}; // 세마포어 얻기
    struct sembuf mysem_close = {0, 1, SEM_UNDO};  // 세마포어 돌려주기

    memset(buf, 0x00, 11);
    memset(count, 0x00, 11);

    // 세마포설정을 한다. 
    semid = semget((key_t)234, 0, 0660|IPC_CREAT);
    if (semid == -1){
    
        perror("semget error ");
        exit(0);
    }    

    printf("semaphore id : %d\n", semid);

    // counter.txt 파일을 열기 위해서 세마포어검사를한다. 
    if(semop(semid, &mysem_open, 1) == -1)
    {
      perror("semop error1 ");
      exit(0);
    }

    /* // counter.txt 파일을 열기 위해서 세마포어검사를한다.  */
    /* if(semop(semid, &mysem_open, 1) == -1) */
    /* { */
    /*   perror("semop error2 "); */
    /*   exit(0); */
    /* } */

    if ((fp = fopen("counter.txt", "r+")) == NULL)
    {
        perror("fopen error ");
        exit(0);
    }
    // 파일의 내용을 읽은후 파일을 처음으로 되돌린다.  
    fgets(buf, 11, fp);
    rewind(fp);

    // 개행문자를 제거한다. 
    buf[strlen(buf) - 1] = 0x00;

    sprintf(count, "%d\n", atoi(buf) + 1); 
    printf("%s", count);
    // 10초를 잠들고 난후 count 를 파일에 쓴다. 
    printf("getchar()");
    getchar();
    fputs(count,fp);

    fclose(fp);
    // 모든 작업을 마쳤다면 세마포어 자원을 되될려준다
    semop(semid, &mysem_close, 1);

    return 1;
}
