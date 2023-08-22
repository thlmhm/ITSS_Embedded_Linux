#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t childPid;
    int status;

    // Task 1: Confirm operation when execv is replaced with other exec * and other Linux commands
    printf("Task 1: Executing 'ls' command using execl\n");
    childPid = fork();
    if (childPid == 0) {
        // Child process
        execl("/bin/ls", "ls", "-l", NULL);
        exit(0);
    } else {
        // Parent process
        wait(&status);
    }

    // Task 2: Confirm the operation when kill pid is executed from the console and the child process is ended
    printf("\nTask 2: Executing 'sleep' command in background and sending SIGTERM to child process\n");
    childPid = fork();
    if (childPid == 0) {
        // Child process
        printf("Child process (PID: %d) is sleeping for 5 seconds\n", getpid());
        sleep(5);
        printf("Child process (PID: %d) woke up\n", getpid());
        exit(0);
    } else {
        // Parent process
        sleep(1); // Give some time for the child process to start
        printf("Sending SIGTERM to child process (PID: %d)\n", childPid);
        kill(childPid, SIGTERM);
        wait(&status);
    }

    // Task 3: Confirm the state of the process of the child process when wait is not used by the parent process
    printf("\nTask 3: Executing 'ps' command to check child process status without using wait\n");
    childPid = fork();
    if (childPid == 0) {
        // Child process
        printf("Child process (PID: %d) is sleeping for 3 seconds\n", getpid());
        sleep(3);
        printf("Child process (PID: %d) woke up\n", getpid());
        exit(0);
    } else {
        // Parent process
        // Do not use wait() to wait for child process
        sleep(1); // Give some time for the child process to start
        printf("Executing 'ps' command to check child process status\n");
        system("ps -o pid,ppid,stat,cmd");
    }

    // Task 4: Make the program that generates and ends two or more child processes
    printf("\nTask 4: Generating and ending two child processes\n");
    for (int i = 0; i < 2; i++) {
        childPid = fork();
        if (childPid == 0) {
            // Child process
            printf("Child process %d (PID: %d) is exiting\n", i + 1, getpid());
            exit(0);
        }
    }

    // Wait for all child processes to exit
    for (int i = 0; i < 2; i++) {
        wait(&status);
    }

    return 0;
}
