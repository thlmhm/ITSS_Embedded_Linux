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

    printf("Program 2 (Message Sender): Start\n");

    keyx = ftok(KEYFILE_PATH, (int)ID);

    // Message queue ID acquisition
    if ((msqid = msgget(keyx, IPC_CREAT | 0660)) == MSGQ_NG) {
        perror("msgget: Program 2 (Message Sender)");
        exit(1);
    }
    printf("Program 2 (Message Sender): Message Queue ID = %d\n", msqid);

    // Output an arbitrary character string
    printf("Program 2 (Message Sender): Enter a message: ");
    fgets(message.mtext, MSGSIZE, stdin);

    // Set the message type
    message.mtype = 1;

    // Send the message
    if (msgsnd(msqid, &message, strlen(message.mtext) + 1, 0) == MSGQ_NG) {
        perror("msgsnd: Program 2 (Message Sender)");
        exit(1);
    }
    printf("Program 2 (Message Sender): Message sent successfully.\n");

    printf("Program 2 (Message Sender): End\n");
    return 0;
}
