#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define KEYFILE_PATH  "abc"
#define ID 'S'
#define SEM_OK 0
#define SEM_NG -1

int main(void)
{
  int semid;
  int semval;
  key_t keyval;
  struct sembuf buff;
  
  printf("Program 1: start\n");

  keyval = ftok(KEYFILE_PATH, (int)ID);

  // Semaphore ID acquisition
  semid = semget(keyval, 1, IPC_CREAT|0660);
  printf("Program 1: semID = %d\n", semid);

  // Sembuf structure setting for semaphore operation
  buff.sem_num = 0;
  buff.sem_op = -1; // Subtract 1 from semaphore value
  buff.sem_flg = 0;

  // Semaphore operation
  semop(semid, &buff, 1);
  printf("Program 1: semaphore value decremented\n");

  // Output an arbitrary character string
  printf("Program 1: Outputting arbitrary character string\n");

  // Control of semaphore (semaphore value acquisition)
  semval = semctl(semid, 0, GETVAL, 0);
  printf("Program 1: semval = %d\n", semval);

  printf("Program 1: end\n");
  return 0;
}
