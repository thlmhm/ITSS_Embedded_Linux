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
#include <sys/wait.h>

#define KEYFILE_PATH "./key.txt"
#define ID 'M'
#define MUTEX_ID 'N'
#define MSGQ_OK 0
#define MSGQ_ERR -1

// a struct for storing message information
struct msgbuff
{
  long mtype;
  int num;
};

void doOperation(int msqid, pthread_mutex_t* pm, int* ptr, int msg_type){
  struct msgbuff message;
  // Receive message
  while (1)
  {
    if ((msgrcv(msqid, &message, sizeof(int), msg_type, 0)) == MSGQ_ERR)
    {
      perror("msgrcv");
      exit(1);
    }

    if (message.num == -1)
    {
      printf("Exit message (-1) received. Exit...\n");
      break;
    }

    // Lock mutex
    if (pthread_mutex_lock(pm) != 0)
    {
      perror("pthread_mutex_lock");
      exit(1);
    }

    printf("\ninput value = %d\n", message.num);
    printf("message type = %d\n", msg_type);

    printf("Mutex locked\n");

    printf("Value of shared memory: (pid=%d) %d\n", getpid(), *ptr);

    sleep(3);

    *ptr = *ptr + message.num;

    printf("Value of shared memory after adding new incoming value: (pid=%d) %d\n", getpid(), *ptr);

    printf("Mutex unlocked\n\n");
    // Unlock mutex
    if (pthread_mutex_unlock(pm) != 0)
    {
      perror("pthread_mutex_unlock");
      exit(1);
    }

    message.mtype = msg_type;
    message.num = *ptr;
    if ((msgsnd(msqid, (void *)&message, sizeof(int), 0)) == MSGQ_ERR)
    {
      perror("msgsnd");
      exit(1);
    }
  }
}

int main(void)
{
  int i;
  int shmid, shmid_mutex;
  int *ptr;
  int value;

  int msqid;
  key_t keyx, key_mutex;
  struct msqid_ds msq;
  pthread_mutexattr_t pmattr;

  keyx = ftok(KEYFILE_PATH, (int)ID);
  key_mutex = ftok(KEYFILE_PATH, (int)MUTEX_ID);
  if (key_mutex == -1) {
      perror("ftok for mutex");
      exit(1);
  }

  if (keyx == -1) {
      perror("ftok for shared mem");
      exit(1);
  }

  // create e message queue for communication
  msqid = msgget(keyx, 0666 | IPC_CREAT);

  if (msqid == -1)
  {
    perror("msgget");
    exit(1);
  }

  // Get the shared memory ID
  if ((shmid = shmget(keyx, sizeof(int), IPC_CREAT | 0660)) == -1)
  {
    perror("shmget shared mem");
    exit(1);
  }

  // Get the shared memory ID
  if ((shmid_mutex = shmget(key_mutex, sizeof(pthread_mutex_t), IPC_CREAT | 0660)) == -1)
  {
    perror("shmget mutex");
    exit(1);
  }

  // Attach the shared memory
  ptr = (int *)shmat(shmid, 0, 0);
  if (ptr == (int *)-1)
  {
    perror("shmat");
    exit(1);
  }

  pthread_mutex_t *pm = (pthread_mutex_t *)shmat(shmid_mutex, 0, 0);

  if (pm == (pthread_mutex_t *)-1)
  {
    perror("shmat mutex");
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

  pid_t pid = 0;                 // process ID
  switch (pid = fork()){
    case -1:
      // fork failed
      perror("processGenerate fork");
      exit(1);
    case 0:
      // Children
      doOperation(msqid, pm, ptr, 1);
      // Detach the shared memory
      if (shmdt(ptr) == -1) {
          perror("shmdt");
          exit(1);
      }
      if (shmdt(pm) == -1) {
          perror("shmdt");
          exit(1);
      }
      wait(NULL);
      return 0;
    default:
      // Parent
      doOperation(msqid, pm, ptr, 2);
      break;
  }

  wait(NULL);

  // Control of message queue (message queue ID deletion)
  if (msgctl(msqid, IPC_RMID, NULL) == MSGQ_ERR)
  {
    perror("msgctl");
  }
  // Destroy Mutex
  pthread_mutex_destroy(pm);

  // Detach the shared memory
  if (shmdt(ptr) == -1) {
    perror("shmdt");
    exit(1);
  }

  if (shmdt(pm) == -1) {
    perror("shmdt mutex");
    exit(1);
  }

  // Remove the shared memory segment
  if (shmctl(shmid, IPC_RMID, NULL) == -1) {
    perror("shmctl");
    exit(1);
  }
  
  if (shmctl(shmid_mutex, IPC_RMID, NULL) == -1) {
    perror("shmctl mutex");
    exit(1);
  }

  return 0;
}