#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

#define MAP_SIZE 100
#define MAP_NAME "MemoryMap"

typedef struct Data
{
    char str[128];
    long lval;
    double dval;
} TestData;

int main()
{
    int pid;
    int fd;
    char c;
    long psize, size;
    TestData *ptr;
    long i, lval;
    double dval;
    char buf[512], *p;

    // Making a file descriptor with the given identity if it doesn't exist
    // Return the file descriptor with the given identity if it exits
    
    if ((fd = open("MapFile", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("open");
        exit(-1);
    }

    // create a memory map for storing 100 instance of TestData
    psize = sysconf(_SC_PAGE_SIZE);
    size = (MAP_SIZE * sizeof(TestData) / psize + 1) * psize;

    // writing to the memory map
    for (i = 0; i < MAP_SIZE; i++)
    {
        TestData data = {"abcd", i, 1.5};
        write(fd, &data, sizeof(TestData));
        printf("Written: %s; %ld; %f\n", data.str, data.lval, data.dval);
        lseek(fd, sizeof(TestData) * (i + 1), SEEK_SET);
    }
    printf("Write finished. Press any key to exit...");
    getchar();
}