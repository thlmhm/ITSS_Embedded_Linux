#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "linkedlist.h"
#include <signal.h>
#include <sys/msg.h>
#include <sys/types.h>

#include <pthread.h>

#define KEYFILE_PATH "./key.txt"
#define ID 'M'
#define ID_mutex 'N'
#define INQUIRY_MSG 1
#define RESULT_MSG 2

#define MSGQ_OK 0
#define MSGQ_ERR -1

int msqid;

typedef struct Process_Struct{
    int process_id; // 1, 2, 3
    int process_pid; // getpid()
} Process;

struct msgbuff
{
  long mtype;
  Process info;
};

int round_robin(int rotate_token){
    if (rotate_token == 3) return 1;
    return rotate_token + 1;
}

int compareProcess(void* p1, void* p2){
    Process* process1 = (Process*) p1;
    Process* process2 = (Process*) p2;
    if (process1->process_id == process2->process_id) return 0;
    return 1;
}

void freeProcess(void* p){
    Process* process = (Process*) p; 
    free(process);
}

Process* cloneProcessFromMessage(struct msgbuff msg){
    Process* process = (Process*) malloc(sizeof(Process));
    process->process_id = msg.info.process_id;
    process->process_pid = msg.info.process_pid;
    return process;
}

Process* findProcessWithID(LinkedList* inquirys, int process_id){
    Process* tempProcess = (Process*) malloc(sizeof(Process));
    tempProcess->process_id = process_id;
    return removeFromList(inquirys, tempProcess);
}

// Signal handler function
void handle_ctrl_c(int signum)
{
    printf("\nCtrl+C pressed. Exiting...\n");
    // Control of message queue (message queue ID deletion)
    if (msgctl(msqid, IPC_RMID, NULL) == MSGQ_ERR)
    {
        perror("msgctl");
    }
    // Exit the program
    exit(signum);
}

int main()
{
    struct msgbuff message;
    int rotate_token = 1;
    key_t keyx;
    
    keyx = ftok(KEYFILE_PATH, (int)ID);

    if (keyx == -1) {
        perror("ftok for shared mem");
        exit(1);
    }


    LinkedList* inquiryProcess = createList(compareProcess, freeProcess);

    // create e message queue for communication between processes
    msqid = msgget(keyx, 0666 | IPC_CREAT);

    if (msqid == -1)
    {
        perror("msgget");
        exit(1);
    }

    signal(SIGINT, handle_ctrl_c);
    printf("accOrderCtrl: Started waiting for message\n");
    printf("msqid: %d\n", msqid);
    fflush(stdout);
    while (1)
    {
        if ((msgrcv(msqid, &message, sizeof(Process), INQUIRY_MSG, 1)) == MSGQ_ERR)
        {
            perror("msgrcv");
            exit(1);
        }
        printf("Receive message from %d:%d code %ld\n", message.info.process_id, message.info.process_pid, message.mtype);
        Process* inquiryEntry = cloneProcessFromMessage(message);
        insertAtEnd(inquiryProcess, inquiryEntry);
        if(message.info.process_id != rotate_token) continue;
        // TODO kill all process.
        do {
            Process* matchProcess = findProcessWithID(inquiryProcess, rotate_token);
            if (matchProcess == NULL) break;
            printf("Send SIGUSR to %d:%d\n", matchProcess->process_id, matchProcess->process_pid);
            int result = kill(matchProcess->process_pid, SIGUSR1);
            if (result != 0){
                perror("kill");
                exit(EXIT_FAILURE);
            }
            // Wait for successfull message
            if ((msgrcv(msqid, &message, sizeof(Process), RESULT_MSG, 0)) == MSGQ_ERR)
            {
                perror("msgrcv");
                exit(1);
            }
            printf("Processing completed message received from %d:%d\n", matchProcess->process_id, matchProcess->process_pid);
            rotate_token = round_robin(rotate_token);
        } while(1);
    }


    return 0;
}