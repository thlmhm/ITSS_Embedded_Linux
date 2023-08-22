#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define FIFO_FILE_PATH "fifoRead.txt"
#define BUFF_SIZE 256

int main()
{
    int fifoFd;
    char buf[BUFF_SIZE];
    ssize_t bytesRead;

    // Open the FIFO for reading only
    if ((fifoFd = open(FIFO_FILE_PATH, O_RDONLY)) == -1)
    {
        perror("fifofile open");
        return 1;
    }

    // Output FIFO file data to console
    while ((bytesRead = read(fifoFd, buf, BUFF_SIZE)) > 0)
    {
        write(STDOUT_FILENO, buf, bytesRead);
    }

    if (bytesRead == -1)
    {
        perror("read from FIFO");
        return 1;
    }

    close(fifoFd);

    return 0;
}
