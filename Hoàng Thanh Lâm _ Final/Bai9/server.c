#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define SERVER_ADDR "localhost"
#define SERVER_PORT 5500

void doprocessing(int iClientSock);

void reverse(char *x, int start, int end);

int main()
{
  int iSock = 0, pid;
  int iClientSock = 0;
  struct sockaddr_in serverData = {0, 0, 0, ""};
  struct sockaddr_in clientData = {0, 0, 0, ""};
  int iClientSize = 0;
  int i, n;

  char buffer[256];

  // Creation of socket
  if ((iSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("socket");
    exit(1);
  }

  // Initialize socket structure 
  bzero((char *)&serverData, sizeof(serverData));

  serverData.sin_family = AF_INET;
  serverData.sin_addr.s_addr = INADDR_ANY;
  serverData.sin_port = htons(SERVER_PORT);

  // Identify the socket
  if (bind(iSock, (struct sockaddr *)&serverData, sizeof(serverData)) == -1)
  {
    perror("bind");
    shutdown(iSock, SHUT_RDWR);
    close(iSock);
    exit(1);
  }

  printf("Bind successfully to local host\n");

  // Connection setup
  if (listen(iSock, 1) == -1)
  {
    perror("listen");
    shutdown(iSock, SHUT_RDWR);
    close(iSock);
    exit(1);
  }

  printf("Start listening...\n");

  iClientSize = sizeof(clientData);

  while (1)
  {
    // Connection standby
    if ((iClientSock = accept(iSock, (struct sockaddr *)&clientData, &iClientSize)) == -1)
    {
      perror("accept");
      shutdown(iSock, SHUT_RDWR);
      close(iSock);
      exit(1);
    }
    // Create child process 
    printf("Get new connection: Client's port: %d\n", clientData.sin_port);
    printf("Start communication\n");
    pid = fork();
    if (pid < 0)
    {
      perror("ERROR on fork");
      exit(1);
    }

    if (pid == 0)
    {
      close(iSock);
      doprocessing(iClientSock);
    }
    else
    {
      close(iClientSock);
    }
  }
  return 0;
}

void doprocessing(int iClientSock)
{
  int i, n;
  char buffer[256];

  while (1)
  {

    // If connection is established then start communicating 
    bzero(buffer, 256);
    n = read(iClientSock, buffer, 255);

    if (n < 0)
    {
      perror("ERROR reading from socket");
      exit(1);
    }

    printf("Message from client: %s\n", buffer);

    if (strncmp(buffer, "quit", 4) == 0)
    {
      printf("Quit message caught. Terminate current session and waiting for new connection\n");
      close(iClientSock);
      exit(0);
    }

    reverse(buffer, 0, strlen(buffer) - 1);

    // Write a response to the client 
    n = write(iClientSock, buffer, strlen(buffer));

    if (n < 0)
    {
      perror("ERROR writing to socket");
      exit(1);
    }
  }
}

void reverse(char *x, int start, int end)
{
  char c;

  if (start >= end)
    return;

  c = *(x + start);
  *(x + start) = *(x + end);
  *(x + end) = c;

  reverse(x, ++start, --end);
}