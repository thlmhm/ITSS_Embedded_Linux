#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

int main()
{
    pthread_mutex_t *pm;
    int fd;

    // Open the shared memory object created by program1
    fd = shm_open("/my_mutex", O_RDWR, 0666);
    if (fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    // Map the shared memory object into the address space
    pm = (pthread_mutex_t *)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pm == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    // Lock mutex
    pthread_mutex_lock(pm);

    // Wait for program1 to unlock the mutex
    pthread_mutex_unlock(pm);
    printf("Mutex unlocked in Program 2\n");

    // Clean up
    munmap(pm, sizeof(pthread_mutex_t));
    close(fd);

    return 0;
}
