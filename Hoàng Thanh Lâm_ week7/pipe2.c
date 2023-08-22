#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define PIPE_READ 0
#define PIPE_WRITE 1
#define PIPE_READ_WRITE 2
#define STD_IN 0
#define STD_OUT 1
#define BUFFSIZE 80


/**
 * parent process
 */
int main(void)
{
    printf("main           : start\n");

    pid_t pid = 0;                 // process ID
    int pipe_c2p[PIPE_READ_WRITE]; // child to parent
    char buf[BUFFSIZE] = {'\0'};
    int writeSize = 0;
    memset(pipe_c2p, 0, sizeof(pipe_c2p));

    // Create pipe
    if (pipe(pipe_c2p) == -1)
    {
        perror("processGenerate pipe");
        exit(1);
    }

    // Create child process
    switch (pid = fork())
    {
    case -1:
        perror("processGenerate fork");
        // Close input/output file descriptor of pipe
        close(pipe_c2p[PIPE_READ]);
        close(pipe_c2p[PIPE_WRITE]);
        exit(1);

    case 0:
        // childProcess(pipe_c2p);
        printf("\nchildProcess start\n");

        char buf[BUFFSIZE] = {'\0'};
        int readSize = 0;

        // read the command line argument from the pipe
        if ((readSize = read(pipe_c2p[PIPE_READ], buf, BUFFSIZE)) == -1)
        {
            perror("childProcess read");
            exit(1);
        }

        // terminate the string
        buf[readSize] = '\0';

        // Switch the file descriptor for the pipe output to a standard input.
        close(STD_IN);
        dup(pipe_c2p[PIPE_READ]);

        // close file descriptor for input/output
        close(pipe_c2p[PIPE_READ]);
        close(pipe_c2p[PIPE_WRITE]);

        // execute the command passed from the parent process
        execlp(buf, buf, NULL);
        // execlp error
        perror("execlp");
        exit(1);
        break;

    default:
        printf("\nparentProcess start\n");

        // get the command line argument from the user
        printf("Enter a command to execute: ");
        scanf("%s", buf);

        // write the command to the pipe
        if ((writeSize = write(pipe_c2p[PIPE_WRITE], buf, strlen(buf))) == -1)
        {
            perror("parentProcess write");
            exit(1);
        }

        // close file descriptor for input/output
        close(pipe_c2p[PIPE_READ]);
        close(pipe_c2p[PIPE_WRITE]);
        // wait for child process to finish
        if (waitpid(pid, NULL, 0) == -1)
        {
            perror("parentProcess waitpid");
            exit(1);
        }
        printf("parentProcess end\n");
        break;
    }
    return 0;
}