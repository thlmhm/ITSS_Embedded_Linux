#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define SIG_OK 0
#define SIG_NG -1
#define LOOP_ON 0
#define LOOP_OFF 1

// Global variables
int loopFlag = LOOP_ON; /* Loop flag */

/**
 * Signal handler function that receives signal SIGALRM
 *
 * param sigNo Received signal number
 *
 * return  none
 */
void sigHandleSigalrm(int sigNo)
{
    printf("Time's up!\n");
    loopFlag = LOOP_OFF;
    return;
}

/**
 * Signal handler function that receives signal SIGINT
 *
 * param sigNo Received signal number
 *
 * return  none
 */
void sigHandleSigint(int sigNo)
{
    printf("Interrupted!\n");
    loopFlag = LOOP_OFF;
    return;
}

/**
 * The main processing
 *
 * param argc The number of arguments
 * param argv The array of arguments
 *
 * return SIG_OK
 */
int main(int argc, char *argv[])
{
    // Set the timer
    int timer = atoi(argv[1]);
    alarm(timer);

    // Signal handler function registration
    signal(SIGALRM, sigHandleSigalrm);
    signal(SIGINT, sigHandleSigint);

    // Loop during the time the loop flag is ON
    while (loopFlag == LOOP_ON)
    {
        printf("Looping\n");
        sleep(1);
    }

    printf("End of program.\n");
    return SIG_OK;
}