#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

int main()
{
  pthread_mutex_t* pm;
  int fd;

  // Create a shared memory object
  fd = shm_open("/my_mutex", O_CREAT | O_RDWR, 0666);
  if (fd == -1) {
    perror("shm_open");
    exit(1);
  }

  // Set the size of the shared memory object
  if (ftruncate(fd, sizeof(pthread_mutex_t)) == -1) {
    perror("ftruncate");
    exit(1);
  }

  // Map the shared memory object into the address space
  pm = (pthread_mutex_t*)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (pm == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  // Initialize the mutex
  pthread_mutexattr_t pmattr;
  pthread_mutexattr_init(&pmattr);
  pthread_mutexattr_setpshared(&pmattr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(pm, &pmattr);

  // Lock mutex
  pthread_mutex_lock(pm);
  printf("Mutex locked in Program 1\n");

  // Wait for user input
  printf("Press Enter to unlock the mutex: ");
  getchar();

  // Unlock mutex
  pthread_mutex_unlock(pm);
  printf("Mutex unlocked in Program 1\n");

  // Clean up
  munmap(pm, sizeof(pthread_mutex_t));
  close(fd);
  shm_unlink("/my_mutex");

  return 0;
}
