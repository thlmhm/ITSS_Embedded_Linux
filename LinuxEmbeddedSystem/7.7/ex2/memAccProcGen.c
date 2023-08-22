#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define KEYFILE_PATH "./key.txt"
#define ID 'M'
#define ID_mutex 'N'

#define INQUIRY_MSG 1
#define RESULT_MSG 2

#define MSGQ_OK 0
#define MSGQ_ERR -1

int semid, msqid, shmid;
int *shared_memory;
int current_id = -1;
int fork_return = -1;

typedef struct Process_Struct{
    int process_id; // 1, 2, 3
    int process_pid; // getpid()
} Process;

struct msgbuff
{
  long mtype;
  Process info;
};

// Signal handler function
void handle_ctrl_c(int signum)
{
    if (fork_return == -1){
        printf("Parent Process release memory!\n");
        // Destroy the Semaphore
        if(semctl(semid, 0, IPC_RMID, 0) == -1){
            perror("semctl");
            exit(1);
        }
        // Detach from the shared memory
        shmdt(shared_memory);
        // Destroy the shared memory
        shmctl(shmid, IPC_RMID, NULL);
        exit(0);
    } else {
        printf("\nCtrl+C pressed. Child process %d Exiting...\n", getpid());
        // Perform any necessary cleanup here
        // Exit the program
        exit(0);
    }
    
}

void sigusr1_handler(int signum) {
    printf("\nReceived SIGUSR1 signal from accOrderCtrl.\n");
    fflush(stdout);
    struct msgbuff message;
    // Lock the binary semaphore

    struct sembuf sem_op;
    sem_op.sem_num = 0;     // Semaphore number (0 for the first semaphore)
    sem_op.sem_op = -1;     // Lock the semaphore (decrement by 1)
    sem_op.sem_flg = SEM_UNDO;
    if (semop(semid, &sem_op, 1) == -1) {
        perror("semop lock");
        exit(1);
    }
    printf("Lock the binary semaphore!\n");
    fflush(stdout);

    switch (current_id)
    {
    case 1:
        // Process 1 do nothing and write X = 10, Y = 0
        printf("Process 1 with pid %d do nothing\n", getpid());
        fflush(stdout);
        shared_memory[0] = 10;
        shared_memory[1] = 0;
        break;
    case 2:
        // Process 2 calculate 100 / X and write X = 0, Y = 10
        printf("Process 2 with pid %d calculate %d\n", getpid(), 100 / shared_memory[0]);
        fflush(stdout);
        shared_memory[0] = 0;
        shared_memory[1] = 10;
        break;
    case 3:
        // Process 3 calculate 10000 / Y and write X = 0, Y = 0
        printf("Process 3 with pid %d calculate %d\n", getpid(), 10000 / shared_memory[1]);
        fflush(stdout);
        shared_memory[0] = 0;
        shared_memory[1] = 0;
        break;
    default:
        break;
    }

    // Unlock the semaphore
    sem_op.sem_op = 1;     // Unlock the semaphore (increment by 1)
    if (semop(semid, &sem_op, 1) == -1) {
        perror("semop lock");
        exit(1);
    }
    printf("Unlock the binary semaphore!\n\n");
    fflush(stdout);

    message.mtype = RESULT_MSG;
    message.info.process_id = current_id;
    message.info.process_pid = getpid();
    if ((msgsnd(msqid, (void *)&message, sizeof(message.info), 0)) == MSGQ_ERR){
        perror("msgsnd");
        exit(1);
    }
}

int main()
{
    key_t keyx;
    keyx = ftok(KEYFILE_PATH, (int)ID);

    if (keyx == -1) {
        perror("ftok for shared mem");
        exit(1);
    }

    // Create semaphore
    semid = semget(keyx, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    unsigned short sem_values[1];
    sem_values[0] = 1;  // Initial value for the semaphore
    if (semctl(semid, 0, SETALL, sem_values) == -1) {
        perror("semctl");
        exit(1);
    }

    msqid = msgget(keyx, 0666 | IPC_CREAT);

    if (msqid == -1)
    {
        perror("msgget");
        exit(1);
    }

    // Create shared memory
    shmid = shmget(keyx, 2 * sizeof(int), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }
    shared_memory = (int *)shmat(shmid, NULL, 0);
    if (shared_memory == (int *)(-1))
    {
        perror("shmat");
        exit(1);
    }
    // Initialize the shared memory instruction
    shared_memory[0] = 0; // X = 0
    shared_memory[1] = 0; // Y = 0

    // Create three child processes to access memory
    for (int i = 1; i <= 3; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        {
            current_id = i;
            fork_return = 0;
            break;
        } 
    }

    // Children
    signal(SIGINT, handle_ctrl_c);
    if (fork_return == 0){
        // Do while true wait
        signal(SIGUSR1, sigusr1_handler);
        // Child process
        srand(time(NULL) + current_id);
        struct msgbuff message;
        printf("Child process %d started\n", getpid());
        while(1){
            int sleepInterval = rand() % 6;
            printf("Process %d:%d sleep %d!\n", current_id, getpid(), sleepInterval);
            sleep(sleepInterval);
            message.mtype = INQUIRY_MSG;
            message.info.process_id = current_id;
            message.info.process_pid = getpid();
            printf("Process %d:%d send inquiry message!\n", current_id, getpid());
            fflush(stdout);
            if ((msgsnd(msqid, (void *)&message, sizeof(message.info), 0)) == MSGQ_ERR){
                perror("msgsnd");
                exit(1);
            }
            pause();
        }
    }

    // printf("Wait NULL\n");
    // fflush(stdout);
    wait(NULL);
    // int status;
    // pid_t wpid;
    // while ((wpid = waitpid(-1, &status, 0)) > 0) {
    //     if (WIFEXITED(status)) {
    //         printf("Child process %d exited with status %d\n", wpid, WEXITSTATUS(status));
    //         fflush(stdout);
    //     } else if (WIFSIGNALED(status)) {
    //         printf("Child process %d terminated by signal %d\n", wpid, WTERMSIG(status));
    //         fflush(stdout);
    //     }
    // }

    return 0;
}