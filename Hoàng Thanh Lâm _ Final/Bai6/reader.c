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

    psize = sysconf(_SC_PAGE_SIZE);
    size = (MAP_SIZE * sizeof(TestData) / psize + 1) * psize;

    // map to the shared memory map using the given file descriptor
    ptr = (TestData *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ((int)ptr == -1)
    {
        perror("mmap");
        exit(-1);
    }
    TestData *ptr1 = ptr;
    for (i = 0; i < MAP_SIZE; i++)
    {
        printf("%s %ld %lf\n", ptr1->str, ptr1->lval, ptr1->dval);
        ptr1++;
    }

    // Synchronize a file with a memory map 
    msync(ptr, size, MS_ASYNC);
    
    // Unmap *
    if (munmap(ptr, size) == -1)
    {
        perror("munmap");
    }
    // Close file descriptor
    close(fd);
    printf("Read finished. Press any key to exit...");
    getchar();
}