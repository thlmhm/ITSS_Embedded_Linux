#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <time.h>

#define SHIMSIZE 256

#define MSGSIZE 100
#define KEYFILE_PATH "filepath"
#define ID 'M'
#define MSGQ_OK 0
#define MSGQ_ERR -1

int main(void)
{
  // a struct for storing message information
  struct msgbuff
  {
    long mtype;
    int num;
  } message;

  int i;
  int shmsize;
  int shmid;
  int *ptr;
  int *head;
  int value;

  int msqid;
  key_t keyx;
  struct msqid_ds msq;

  // a small mutex for prevent 2 processes to acess the shared memory at the same time
  // which can cause the result errors.
  pthread_mutex_t *pm = malloc(sizeof(pthread_mutex_t));
  pthread_mutexattr_t pmattr;

  keyx = ftok(KEYFILE_PATH, (int)ID);

  // create e message queue for communication
  msqid = msgget(keyx, 0666 | IPC_CREAT);

  if (msqid == -1)
  {
    perror("msgget");
    exit(1);
  }

  shmsize = SHIMSIZE;

  //Get the shared memory ID
  if ((shmid = shmget(keyx, shmsize * sizeof(int), IPC_CREAT | 0660)) == -1)
  {
    perror("shmget");
    exit(1);
  }

  // printf("shmid: %d\n", shmid);

  //Attach the shared memory
  head = ptr = (int *)shmat(shmid, 0, 0);
  if (head == (int *)-1)
  {
    perror("shmat");
    exit(1);
  }

  *ptr = 0;

  printf("Address of shared memory: %p\n", ptr);

  // Initialize the mutex attribute object
  pthread_mutexattr_init(&pmattr);

  // Set mutex attribute (Setting for using mutex to inter process)
  if (pthread_mutexattr_setpshared(&pmattr, PTHREAD_PROCESS_SHARED) != 0)
  {
    perror("pthread_mutexattr_setpshared");
    exit(1);
  }
  // Initialize mutex
  pthread_mutex_init(pm, &pmattr);


  //Receive message
  while (1)
  {
    if ((msgrcv(msqid, &message, sizeof(int), 1, 0)) ==
        MSGQ_ERR)
    {
      perror("msgrcv");
      exit(1);
    }
    printf("input value = %d\n", message.num);

    if (message.num == -1)
    {
      printf("Exit message (-1) received. Press any key to exit...");
      getchar();
      break;
    }

    // Lock mutex
    if (pthread_mutex_lock(pm) != 0)
    {
      perror("pthread_mutex_lock");
      exit(1);
    }

    printf("Mutex locked for prevent harassing\n");

    printf("Value of shared memory: (pid=%d) %d\n", getpid(), *ptr);

    sleep(3);

    *ptr = *ptr + message.num;

    printf("Value of shared memory after adding new incoming value: (pid=%d) %d\n", getpid(), *ptr);

    // Unlock mutex
    if (pthread_mutex_unlock(pm) != 0)
    {
      perror("pthread_mutex_unlock");
      exit(1);
    }
    printf("Mutex unlocked\n");

    message.mtype = 1;
    message.num = *ptr;
    if ((msgsnd(msqid, (void *)&message, sizeof(int), 0)) == MSGQ_ERR)
    {
      perror("msgsnd");
      exit(1);
    }
  }

  //Control of message queue (message queue ID deletion)
  if (msgctl(msqid, IPC_RMID, NULL) == MSGQ_ERR)
  {
    perror("msgctl");
  }

  return 0;
}