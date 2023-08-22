#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PIPE_READ 0
#define PIPE_WRITE 1
#define PIPE_READ_WRITE 2
#define STD_IN 0
#define STD_OUT 1
#define BUFFSIZE 80

#define MSGSIZE 100
#define KEYFILE_PATH "filepath"
#define ID 'M'
#define MSGQ_OK 0
#define MSGQ_ERR -1

/**
 * main process
 */

int main(void)
{
  // a struct for storing message information. This is the same as in program1.c
  struct msgbuff
  {
    long mtype;
    int num;
  } message;

  int msqid;
  key_t keyx;
  struct msqid_ds msq;

  pid_t pid = 0;                 // process ID
  int pipe_c2p[PIPE_READ_WRITE]; // child to parent
  int num;
  int readSize = 0;

  memset(pipe_c2p, 0, sizeof(pipe_c2p));

  // Create a pipe for transfering data
  if (pipe(pipe_c2p) == -1)
  {
    perror("processGenerate pipe");
    exit(1);
  }

  keyx = ftok(KEYFILE_PATH, (int)ID);

  msqid = msgget(keyx, 0666 | IPC_CREAT);

  if (msqid == -1)
  {
    perror("msgget");
    exit(1);
  }

  //fork();

  // create child process
  switch (pid = fork())
  {
  case -1:
    // fork failed
    perror("processGenerate fork");
    // close file descriptor for input/output
    close(pipe_c2p[PIPE_READ]);
    close(pipe_c2p[PIPE_WRITE]);
    exit(1);

  case 0:
    // close file descriptor for output
    close(pipe_c2p[PIPE_WRITE]);

    while (1)
    {
      // read from parent process
      if ((readSize = read(pipe_c2p[PIPE_READ], &num, sizeof(int))) != 0)
      {
        message.mtype = 1;
        message.num = num;
        if ((msgsnd(msqid, (void *)&message, sizeof(int), 0)) ==
            MSGQ_ERR)
        {
          perror("msgsnd");
          exit(1);
        }
        if (message.num == -1)
        {
          break;
        }
        if ((msgrcv(msqid, &message, sizeof(int), 1, 0)) ==
            MSGQ_ERR)
        {
          perror("msgrcv");
          exit(1);
        }
      }
    }
    // close file descriptor for input
    close(pipe_c2p[PIPE_READ]);
    break;
  default:
    // close file descriptor for input
    close(pipe_c2p[PIPE_READ]);

    while (1)
    {
      printf("Current pid: %d\nEnter number to send (-1 to exit the program): ", getpid());
      scanf("%d", &num);
      write(pipe_c2p[PIPE_WRITE], &num, sizeof(int));
      if (num == -1)
      {
        break;
      }
    }

    // close file descriptor for output
    close(pipe_c2p[PIPE_WRITE]);
    exit(0);
  }
  return 0;
}