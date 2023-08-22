#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define FIFO_FILE_PATH "fifoRead.txt"
#define FILE_TO_READ_PATH "fifoOut.txt"
#define BUFF_SIZE 256

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    const char *fileToReadPath = argv[1];
    int fileToReadFd, fifoFd;
    char buf[BUFF_SIZE];
    ssize_t bytesRead, bytesWritten;

    // Open the file to read
    if ((fileToReadFd = open(fileToReadPath, O_RDONLY)) == -1)
    {
        perror("filetoread open");
        return 1;
    }

    // Open the FIFO for writing only
    if ((fifoFd = open(FIFO_FILE_PATH, O_WRONLY)) == -1)
    {
        perror("fifofile open");
        return 1;
    }

    // Output file data to the FIFO
    while ((bytesRead = read(fileToReadFd, buf, BUFF_SIZE)) > 0)
    {
        bytesWritten = write(fifoFd, buf, bytesRead);
        if (bytesWritten == -1)
        {
            perror("write to FIFO");
            return 1;
        }
    }

    if (bytesRead == -1)
    {
        perror("read from file");
        return 1;
    }

    close(fileToReadFd);
    close(fifoFd);

    return 0;
}
