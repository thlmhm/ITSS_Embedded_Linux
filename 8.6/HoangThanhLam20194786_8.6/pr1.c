#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MSGSIZE 100
#define KEYFILE_PATH "keyfilepath"
#define ID 'M'
#define MSGQ_OK 0
#define MSGQ_NG -1

int main(void)
{
    struct msgbuff {
        long mtype;
        char mtext[MSGSIZE];
    } message;

    int msqid;
    key_t keyx;
    struct msqid_ds msq;

    printf("Program 1 (Message Receiver): Start\n");

    keyx = ftok(KEYFILE_PATH, (int)ID);

    // Message queue ID acquisition
    if ((msqid = msgget(keyx, IPC_CREAT | 0660)) == MSGQ_NG) {
        perror("msgget: Program 1 (Message Receiver)");
        exit(1);
    }
    printf("Program 1 (Message Receiver): Message Queue ID = %d\n", msqid);

    // Receive message
    if (msgrcv(msqid, &message, sizeof(message.mtext), 1, 0) == MSGQ_NG) {
        perror("msgrcv: Program 1 (Message Receiver)");
        exit(1);
    }
    printf("Program 1 (Message Receiver): Received message = %s\n", message.mtext);

    // Control of message queue (message queue ID deletion)
    if (msgctl(msqid, IPC_RMID, NULL) == MSGQ_NG) {
        perror("msgctl: Program 1 (Message Receiver)");
        exit(1);
    }

    printf("Program 1 (Message Receiver): End\n");
    return 0;
}
