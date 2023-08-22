#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

// Global variable
volatile int stopFlag = 0;

// Signal handler function for SIGALRM
void sigHandleSigalrm(int sigNo)
{
    stopFlag = 1;
}

void sleepAccuracyTest(unsigned int seconds, unsigned int microseconds)
{
    struct timespec start, end;
    double elapsedTime;

    // Get current time
    clock_gettime(CLOCK_REALTIME, &start);

    // Set up signal handler for SIGALRM
    signal(SIGALRM, sigHandleSigalrm);
    alarm(seconds);
    
    struct timespec delay;
    delay.tv_sec = seconds;
    delay.tv_nsec = microseconds * 1000;
    nanosleep(&delay, NULL);

    // Get end time
    clock_gettime(CLOCK_REALTIME, &end);

    // Calculate elapsed time in milliseconds
    elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsedTime += (end.tv_nsec - start.tv_nsec) / 1000000.0;

    // Print results
    printf("Elapsed time: %.3f ms\n", elapsedTime);
}

int main()
{
    // Test sleep(), usleep(), and nanosleep() with different sleep times
    printf("Testing sleep() with 1 second...\n");
    sleepAccuracyTest(1, 0);

    printf("Testing usleep() with 500,000 microseconds...\n");
    sleepAccuracyTest(0, 500000);

    printf("Testing nanosleep() with 1 second and 500,000 microseconds...\n");
    sleepAccuracyTest(1, 500000);

    // Check operation with SIGALRM during sleep
    printf("Testing SIGALRM during sleep...\n");
    stopFlag = 0;
    signal(SIGALRM, sigHandleSigalrm);
    alarm(3);
    printf("Sleeping for 5 seconds...\n");
    sleep(5);
    if (stopFlag)
    {
        printf("Received SIGALRM during sleep\n");
    }
    else
    {
        printf("Sleep completed without interruption\n");
    }

    return 0;
}
